#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import B2DEBUG, B2ERROR, B2INFO, B2WARNING
import re
import os
from abc import ABC, abstractmethod
import json
import xml.etree.ElementTree as ET
from math import ceil
from pathlib import Path
from collections import deque
from itertools import count, takewhile
import shutil
import time
from datetime import datetime, timedelta
import subprocess
import multiprocessing as mp

from caf.utils import method_dispatch
from caf.utils import decode_json_string
from caf.utils import grouper
from caf.utils import parse_file_uri


__all__ = ["Job", "SubJob", "Backend", "Local", "Batch", "LSF", "PBS", "HTCondor", "get_input_data"]

#: Default name of the input data file passed to Backend jobs
_input_data_file_path = Path("__BACKEND_INPUT_FILES__.json")
#: stdout file name
_STDOUT_FILE = "stdout"
#: stderr file name
_STDERR_FILE = "stderr"


def get_input_data():
    """
    Simple JSON load of the default input data file. Will contain a list of string file paths
    for use by the job process.
    """
    with open(_input_data_file_path, mode="r") as input_data_file:
        input_data = json.load(input_data_file)
    return input_data


def monitor_jobs(args, jobs):
    unfinished_jobs = jobs[:]
    while unfinished_jobs:
        B2INFO(f"Updating statuses of unfinished jobs...")
        for j in unfinished_jobs:
            j.update_status()
        B2INFO(f"Checking if jobs are ready...")
        for j in unfinished_jobs[:]:
            if j.ready():
                B2INFO(f"{j} is finished")
                unfinished_jobs.remove(j)
        if unfinished_jobs:
            B2INFO(f"Not all jobs done yet, waiting {args.heartbeat} seconds before re-checking...")
            time.sleep(args.heartbeat)
    B2INFO(f"All jobs finished")


class ArgumentsGenerator():
    def __init__(self, generator_function, *args, **kwargs):
        """
        Simple little class to hold a generator (uninitialised) and the necessary args/kwargs to
        initialise it. This lets us re-use a generator by setting it up again fresh. This is not
        optimal for expensive calculations, but it is nice for making large sequences of
        Job input arguments on the fly.

        Parameters:
            generator_function (py:function): A function (callable) that contains a ``yield`` statement. This generator
                should *not* be initialised i.e. you haven't called it with ``generator_function(*args, **kwargs)``
                yet. That will happen when accessing the `ArgumentsGenerator.generator` property.
            args (tuple): The positional arguments you want to send into the intialisation of the generator.
            kwargs (dict): The keyword arguments you want to send into the intialisation of the generator.
        """
        #: Generator function that has not been 'primed'.
        self.generator_function = generator_function
        #: Positional argument tuple used to 'prime' the `ArgumentsGenerator.generator_function`.
        self.args = args
        #: Keyword argument dictionary used to 'prime' the `ArgumentsGenerator.generator_function`.
        self.kwargs = kwargs

    @property
    def generator(self):
        """
        Returns:
            generator: The initialised generator (using the args and kwargs for initialisation). It should be ready
            to have ``next``/``send`` called on it.
        """
        gen = self.generator_function(*self.args, **self.kwargs)
        gen.send(None)  # Prime it
        return gen


def range_arguments(start=0, stop=None, step=1):
    """
    A simple example Arguments Generator function for use as a `ArgumentsGenerator.generator_function`.
    It will return increasing values using itertools.count. By default it is infinite and will not call `StopIteration`.
    The `SubJob` object is sent into this function with `send` but is not used.

    Parameters:
        start (int): The starting value that will be returned.
        stop (int): At this value the `StopIteration` will be thrown. If this is `None` then this generator will continue
            forever.
        step (int): The step size.

    Returns:
        tuple
    """
    def should_stop(x):
        if stop is not None and x >= stop:
            return True
        else:
            return False
    # The logic of send/yield is tricky. Basically the first yield must have been hit before send(subjob) can work.
    # However the first yield is also the one that receives the send(subjob) NOT the send(None).
    subjob = (yield None)  # Handle the send(None) and first send(subjob)
    args = None
    for i in takewhile(lambda x: not should_stop(x), count(start, step)):
        args = (i,)  # We could have used the subjob object here
        B2DEBUG(29, f"{subjob} arguments will be {args}")
        subjob = (yield args)  # Set up ready for the next loop (if it happens)


class SubjobSplitter(ABC):
    """
    Abstract base class. This class handles the logic of creating subjobs for a `Job` object.
    The `create_subjobs` function should be implemented and used to construct
    the subjobs of the parent job object.

    Parameters:
        arguments_generator (ArgumentsGenerator): Used to construct the generator function that will yield the argument
            tuple for each `SubJob`. The splitter will iterate through the generator each time `create_subjobs` is
            called. The `SubJob` will be sent into the generator with ``send(subjob)`` so that the generator can decide what
            arguments to return.
    """

    def __init__(self, *, arguments_generator=None):
        """
        Derived classes should call `super` to run this.
        """
        #: The `ArgumentsGenerator` used when creating subjobs.
        self.arguments_generator = arguments_generator

    @abstractmethod
    def create_subjobs(self, job):
        """
        Implement this method in derived classes to generate the `SubJob` objects.
        """

    def assign_arguments(self, job):
        """
        Use the `arguments_generator` (if one exists) to assign the argument tuples to the
        subjobs.
        """
        if self.arguments_generator:
            arg_gen = self.arguments_generator.generator
            generating = True
            for subjob in sorted(job.subjobs.values(), key=lambda sj: sj.id):
                if generating:
                    try:
                        args = arg_gen.send(subjob)
                    except StopIteration:
                        B2ERROR((f"StopIteration called when getting args for {subjob}, "
                                 "setting all subsequent subjobs to have empty argument tuples."))
                        args = tuple()  # If our generator finishes before the subjobs we use empty argument tuples
                        generating = False
                else:
                    args = tuple()
                B2DEBUG(29, f"Arguments for {subjob}: {args}")
                subjob.args = args
            return

        B2INFO((f"No ArgumentsGenerator assigned to the {self} so subjobs of {job} "
                "won't automatically have arguments assigned."))

    def __repr__(self):
        return f"{self.__class__.__name__}"


class MaxFilesSplitter(SubjobSplitter):

    def __init__(self, *, arguments_generator=None, max_files_per_subjob=1):
        """
        Parameters:
            max_files_per_subjob (int): The maximium number of input files used per `SubJob` created.
        """
        super().__init__(arguments_generator=arguments_generator)
        #: The maximium number of input files that will be used for each `SubJob` created.
        self.max_files_per_subjob = max_files_per_subjob

    def create_subjobs(self, job):
        """
        This function creates subjobs for the parent job passed in. It creates as many subjobs as required
        in order to prevent the number of input files per subjob going over the limit set by
        `MaxFilesSplitter.max_files_per_subjob`.
        """
        if not job.input_files:
            B2WARNING(f"Subjob splitting by input files requested, but no input files exist for {job}. No subjobs created.")
            return

        for i, subjob_input_files in enumerate(grouper(self.max_files_per_subjob, job.input_files)):
            subjob = job.create_subjob(i, input_files=subjob_input_files)

        self.assign_arguments(job)

        B2INFO(f"{self} created {i+1} Subjobs for {job}")


class MaxSubjobsSplitter(SubjobSplitter):

    def __init__(self, *, arguments_generator=None, max_subjobs=1000):
        """
        Parameters:
            max_subjobs (int): The maximium number ofsubjobs that will be created.
        """
        super().__init__(arguments_generator=arguments_generator)
        #: The maximum number of `SubJob` objects to be created, input files are split evenly between them.
        self.max_subjobs = max_subjobs

    def create_subjobs(self, job):
        """
        This function creates subjobs for the parent job passed in. It creates as many subjobs as required
        by the number of input files up to the maximum set by `MaxSubjobsSplitter.max_subjobs`. If there are
        more input files than `max_subjobs` it instead groups files by the minimum number per subjob in order to
        respect the subjob limit e.g. If you have 11 input files and a maximum number of subjobs of 4, then it
        will create 4 subjobs, 3 of them with 3 input files, and one with 2 input files.
        """
        if not job.input_files:
            B2WARNING(f"Subjob splitting by input files requested, but no input files exist for {job}. No subjobs created.")
            return

        # Since we will be popping from the left of the input file list, we construct a deque
        remaining_input_files = deque(job.input_files)
        # The initial number of available subjobs, will go down as we create them
        available_subjobs = self.max_subjobs
        subjob_i = 0
        while remaining_input_files:
            # How many files should we use for this subjob?
            num_input_files = ceil(len(remaining_input_files)/available_subjobs)
            # Pop them from the remaining files
            subjob_input_files = []
            for i in range(num_input_files):
                subjob_input_files.append(remaining_input_files.popleft())
            # Create the actual subjob
            subjob = job.create_subjob(subjob_i, input_files=subjob_input_files)
            subjob_i += 1
            available_subjobs -= 1

        self.assign_arguments(job)
        B2INFO(f"{self} created {subjob_i} Subjobs for {job}")


class ArgumentsSplitter(SubjobSplitter):
    """
    Creates SubJobs based on the given argument generator. The generator will be called until a `StopIteration` is issued.
    Be VERY careful to not accidentally give an infinite generator! Otherwise it will simply create SubJobs until you run out
    of memory. You can set the `ArgumentsSplitter.max_subjobs` parameter to try and prevent this and throw an exception.

    This splitter is useful for MC production jobs where you don't have any input files, but you want to control the exp/run
    numbers of subjobs. If you do have input files set for the parent `Job` objects, then the same input files will be
    assinged to every `SubJob`.

    Parameters:
        arguments_generator (ArgumentsGenerator): The standard ArgumentsGenerator that is used to assign arguments
    """

    def __init__(self, *, arguments_generator=None, max_subjobs=None):
        """
        """
        super().__init__(arguments_generator=arguments_generator)
        #: If we try to create more than this many subjobs we throw an exception, if None then there is no maximum.
        self.max_subjobs = max_subjobs

    def create_subjobs(self, job):
        """
        This function creates subjobs for the parent job passed in. It creates subjobs until the
        `SubjobSplitter.arguments_generator` finishes.

        If `ArgumentsSplitter.max_subjobs` is set, then it will throw an exception if more than this number of
        subjobs are created.
        """
        arg_gen = self.arguments_generator.generator
        for i in count():
            # Reached the maximum?
            if i >= self.max_subjobs:
                raise SplitterError(f"{self} tried to create more subjobs than the maximum (={self.max_subjobs}).")
            try:
                subjob = SubJob(job, i, job.input_files)  # We manually create it because we might not need it
                args = arg_gen.send(subjob)  # Might throw StopIteration
                B2INFO(f"Creating {job}.{subjob}")
                B2DEBUG(29, f"Arguments for {subjob}: {args}")
                subjob.args = args
                job.subjobs[i] = subjob
            except StopIteration:
                break
        B2INFO(f"{self} created {i+1} Subjobs for {job}")


class Job:
    """
    This generic Job object is used to tell a Backend what to do.
    This object basically holds necessary information about a process you want to submit to a `Backend`.
    It should *not* do anything that is backend specific, just hold the configuration for a job to be
    successfully submitted and monitored using a backend. The result attribute is where backend
    specific job monitoring goes.

    Parameters:
        name (str): Simply a name to describe the Job, not used for any critical purpose in the CAF

    .. warning:: It is recommended to always use absolute paths for files when submitting a `Job`.
    """

    #: Allowed Job status dictionary. The  key is the status name and the value is its level. The lowest level
    #: out of all subjobs is the one that is the overal status of the overall job.
    statuses = {"init": 0, "submitted": 1, "running": 2, "failed": 3, "completed": 4}

    #: Job statuses that correspond to the Job being finished (successfully or not)
    exit_statuses = ["failed", "completed"]

    def __init__(self, name, job_dict=None):
        """
        """
        #: Job object's name. Only descriptive, not necessarily unique.
        self.name = name
        #: The `SubjobSplitter` used to create subjobs if necessary
        self.splitter = None

        if not job_dict:
            #: Files to be copied directly into the working directory (`pathlib.Path`).
            #  Not the input root files, those should be in `Job.input_files`.
            self.input_sandbox_files = []
            #: Working directory of the job (`pathlib.Path`). Default is '.', mostly used in Local() backend
            self.working_dir = Path()
            #: Output directory (`pathlib.Path`), where we will download our output_files to. Default is '.'
            self.output_dir = Path()
            #: Files that we produce during the job and want to be returned. Can use wildcard (*)
            self.output_patterns = []
            #: Command and arguments as a list that wil be run by the job on the backend
            self.cmd = []
            #: The arguments that will be applied to the `cmd` (These are ignored by SubJobs as they have their own arguments)
            self.args = []
            #: Input files to job (`str`), a list of these is copied to the working directory.
            self.input_files = []
            #: Bash commands to run before the main self.cmd (mainly used for batch system setup)
            self.setup_cmds = []
            #: Config dictionary for the backend to use when submitting the job.
            #: Saves us from having multiple attributes that may or may not be used.
            self.backend_args = {}
            #: dict of subjobs assigned to this job
            self.subjobs = {}
        elif job_dict:
            self.input_sandbox_files = [Path(p) for p in job_dict["input_sandbox_files"]]
            self.working_dir = Path(job_dict["working_dir"])
            self.output_dir = Path(job_dict["output_dir"])
            self.output_patterns = job_dict["output_patterns"]
            self.cmd = job_dict["cmd"]
            self.args = job_dict["args"]
            self.input_files = job_dict["input_files"]
            self.setup_cmds = job_dict["setup_cmds"]
            self.backend_args = job_dict["backend_args"]
            self.subjobs = {}
            for subjob_dict in job_dict["subjobs"]:
                self.create_subjob(subjob_dict["id"], input_files=subjob_dict["input_files"], args=subjob_dict["args"])

        #: The result object of this Job. Only filled once the job is submitted to a backend since the backend creates a special
        #: result class depending on its type.
        self.result = None
        #: The actual status of the overall `Job`. The property handles querying for the subjob status to set this
        self._status = "init"

    def __repr__(self):
        """
        Representation of Job class (what happens when you print a Job() instance).
        """
        return f"Job({self.name})"

    def ready(self):
        """
        Returns whether or not the Job has finished. If the job has subjobs then it will return true when they are all finished.
        It will return False as soon as it hits the first failure. Meaning that you cannot guarantee that all subjobs will have
        their status updated when calling this method. Instead use :py:meth:`update_status` to update all statuses if necessary.
        """
        if not self.result:
            B2DEBUG(29, f"You requested the ready() status for {self} but there is no result object set, returning False.")
            return False
        else:
            return self.result.ready()

    def update_status(self):
        """
        Calls :py:meth:`update_status` on the job's result. The result object should update all of the subjobs (if there are any)
        in the best way for the type of result object/backend.
        """
        if not self.result:
            B2DEBUG(29, f"You requested update_status() for {self} but there is no result object set yet. Probably not submitted.")
        else:
            self.result.update_status()
        return self.status

    def create_subjob(self, i, input_files=None, args=None):
        """
        Creates a subjob Job object that references that parent Job.
        Returns the SubJob object at the end.
        """
        if i not in self.subjobs:
            B2INFO(f"Creating {self}.Subjob({i})")
            subjob = SubJob(self, i, input_files)
            if args:
                subjob.args = args
            self.subjobs[i] = subjob
            return subjob
        else:
            B2WARNING(f"{self} already contains SubJob({i})! This will not be created.")

    @property
    def status(self):
        """
        Returns the status of this Job. If the job has subjobs then it will return the overall status equal to the lowest
        subjob status in the hierarchy of statuses in `Job.statuses`.
        """
        if self.subjobs:
            job_status = self._get_overall_status_from_subjobs()
            if job_status != self._status:
                #: Not a real attribute, it's a property
                self.status = job_status
        return self._status

    def _get_overall_status_from_subjobs(self):
        subjob_statuses = [subjob.status for subjob in self.subjobs.values()]
        status_level = min([self.statuses[status] for status in subjob_statuses])
        for status, level in self.statuses.items():
            if level == status_level:
                return status

    @status.setter
    def status(self, status):
        """
        Sets the status of this Job.
        """
        # Print an error only if the job failed.
        if status == self.statuses[3]:
            B2ERROR(f"Setting {self.name} status to {status}")
        else:
            B2INFO(f"Setting {self.name} status to {status}")
        self._status = status

    @property
    def output_dir(self):
        return self._output_dir

    @output_dir.setter
    def output_dir(self, value):
        self._output_dir = Path(value).absolute()

    @property
    def working_dir(self):
        return self._working_dir

    @working_dir.setter
    def working_dir(self, value):
        self._working_dir = Path(value).absolute()

    @property
    def input_sandbox_files(self):
        return self._input_sandbox_files

    @input_sandbox_files.setter
    def input_sandbox_files(self, value):
        self._input_sandbox_files = [Path(p).absolute() for p in value]

    @property
    def input_files(self):
        return self._input_files

    @input_files.setter
    def input_files(self, value):
        self._input_files = value

    @property
    def max_subjobs(self):
        return self.splitter.max_subjobs

    @max_subjobs.setter
    def max_subjobs(self, value):
        self.splitter = MaxSubjobsSplitter(max_subjobs=value)
        B2DEBUG(29, f"Changed splitter to {self.splitter} for {self}.")

    @property
    def max_files_per_subjob(self):
        return self.splitter.max_files_per_subjob

    @max_files_per_subjob.setter
    def max_files_per_subjob(self, value):
        self.splitter = MaxFilesSplitter(max_files_per_subjob=value)
        B2DEBUG(29, f"Changed splitter to {self.splitter} for {self}.")

    def dump_to_json(self, file_path):
        """
        Dumps the Job object configuration to a JSON file so that it can be read in again later.

        Parameters:
          file_path(`Path`): The filepath we'll dump to
        """
        with open(file_path, mode="w") as job_file:
            json.dump(self.job_dict, job_file, indent=2)

    @classmethod
    def from_json(cls, file_path):
        with open(file_path, mode="r") as job_file:
            job_dict = json.load(job_file)
        return cls(job_dict["name"], job_dict=job_dict)

    @property
    def job_dict(self):
        """
        Returns:
            dict: A JSON serialisable representation of the `Job` and its `SubJob` objects. `Path` objects are converted to
            string via ``Path.as_posix()``.
        """
        job_dict = {}
        job_dict["name"] = self.name
        job_dict["input_sandbox_files"] = [i.as_posix() for i in self.input_sandbox_files]
        job_dict["working_dir"] = self.working_dir.as_posix()
        job_dict["output_dir"] = self.output_dir.as_posix()
        job_dict["output_patterns"] = self.output_patterns
        job_dict["cmd"] = self.cmd
        job_dict["args"] = self.args
        job_dict["input_files"] = self.input_files
        job_dict["setup_cmds"] = self.setup_cmds
        job_dict["backend_args"] = self.backend_args
        job_dict["subjobs"] = [sj.job_dict for sj in self.subjobs.values()]
        return job_dict

    def dump_input_data(self):
        """
        Dumps the `Job.input_files` attribute to a JSON file. input_files should be a list of
        string URI objects.
        """
        with open(Path(self.working_dir, _input_data_file_path), mode="w") as input_data_file:
            json.dump(self.input_files, input_data_file, indent=2)

    def copy_input_sandbox_files_to_working_dir(self):
        """
        Get all of the requested files for the input sandbox and copy them to the working directory.
        Files like the submit.sh or input_data.json are not part of this process.
        """
        for file_path in self.input_sandbox_files:
            if file_path.is_dir():
                shutil.copytree(file_path, Path(self.working_dir, file_path.name))
            else:
                shutil.copy(file_path, self.working_dir)

    def check_input_data_files(self):
        """
        Check the input files and make sure that there aren't any duplicates.
        Also check if the files actually exist if possible.
        """
        existing_input_files = []  # We use a list instead of set to avoid losing any ordering of files
        for file_path in self.input_files:
            file_uri = parse_file_uri(file_path)
            if file_uri.scheme == "file":
                p = Path(file_uri.path)
                if p.is_file():
                    if file_uri.geturl() not in existing_input_files:
                        existing_input_files.append(file_uri.geturl())
                    else:
                        B2WARNING(f"Requested input file path {file_path} was already added, skipping it.")
                else:
                    B2WARNING(f"Requested input file path {file_path} does not exist, skipping it.")
            else:
                B2DEBUG(29, f"{file_path} is not a local file URI. Skipping checking if file exists")
                if file_path not in existing_input_files:
                    existing_input_files.append(file_path)
                else:
                    B2WARNING(f"Requested input file path {file_path} was already added, skipping it.")
        if self.input_files and not existing_input_files:
            B2WARNING(f"No valid input file paths found for {job}, but some were requested.")

        # Replace the Job's input files with the ones that exist + duplicates removed
        self.input_files = existing_input_files

    @property
    def full_command(self):
        """
        Returns:
            str: The full command that this job will run including any arguments.
        """
        all_components = self.cmd[:]
        all_components.extend(self.args)
        # We do a convert to string just in case arguments were generated as different types
        full_command = " ".join(map(str, all_components))
        B2DEBUG(29, f"Full command of {self} is '{full_command}'")
        return full_command

    def append_current_basf2_setup_cmds(self):
        """
        This adds simple setup commands like ``source /path/to/tools/b2setup`` to your `Job`.
        It should detect if you are using a local release or CVMFS and append the correct commands
        so that the job will have the same basf2 release environment. It should also detect
        if a local release is not compiled with the ``opt`` option.

        Note that this *doesn't mean that every environment variable is inherited* from the submitting
        process environment.
        """
        if "BELLE2_TOOLS" not in os.environ:
            raise BackendError("No BELLE2_TOOLS found in environment")
        if "BELLE2_RELEASE" in os.environ:
            self.setup_cmds.append(f"source {os.environ['BELLE2_TOOLS']}/b2setup {os.environ['BELLE2_RELEASE']}")
        elif 'BELLE2_LOCAL_DIR' in os.environ:
            self.setup_cmds.append("export BELLE2_NO_TOOLS_CHECK=\"TRUE\"")
            self.setup_cmds.append(f"BACKEND_B2SETUP={os.environ['BELLE2_TOOLS']}/b2setup")
            self.setup_cmds.append(f"BACKEND_BELLE2_RELEASE_LOC={os.environ['BELLE2_LOCAL_DIR']}")
            self.setup_cmds.append(f"BACKEND_BELLE2_OPTION={os.environ['BELLE2_OPTION']}")
            self.setup_cmds.append(f"pushd $BACKEND_BELLE2_RELEASE_LOC > /dev/null")
            self.setup_cmds.append(f"source $BACKEND_B2SETUP")
            # b2code-option has to be executed only after the source of the tools.
            self.setup_cmds.append(f"b2code-option $BACKEND_BELLE2_OPTION")
            self.setup_cmds.append(f"popd > /dev/null")


class SubJob(Job):
    """
    This mini-class simply holds basic information about which subjob you are
    and a reference to the parent Job object to be able to access the main data there.
    Rather than replicating all of the parent job's configuration again.
    """

    def __init__(self, job, subjob_id, input_files=None):
        """
        """
        #: Id of Subjob
        self.id = subjob_id
        #: Job() instance of parent to this SubJob
        self.parent = job
        #: Input files specific to this subjob
        if not input_files:
            input_files = []
        self.input_files = input_files
        #: The result object of this SubJob. Only filled once it is is submitted to a backend
        #: since the backend creates a special result class depending on its type.
        self.result = None
        #: Status of the subjob
        self._status = "init"
        #: Arguments specific to this SubJob
        self.args = []

    @property
    def output_dir(self):
        """
        Getter for output_dir of SubJob. Accesses the parent Job output_dir to infer this."""
        return Path(self.parent.output_dir, str(self.id))

    @property
    def working_dir(self):
        """Getter for working_dir of SubJob. Accesses the parent Job working_dir to infer this."""
        return Path(self.parent.working_dir, str(self.id))

    @property
    def name(self):
        """Getter for name of SubJob. Accesses the parent Job name to infer this."""
        return "_".join((self.parent.name, str(self.id)))

    @property
    def status(self):
        """
        Returns the status of this SubJob.
        """
        return self._status

    @status.setter
    def status(self, status):
        """
        Sets the status of this Job.
        """
        # Print an error only if the job failed.
        if status == self.statuses[3]:
            B2ERROR(f"Setting {self.name} status to {status}")
        else:
            B2INFO(f"Setting {self.name} status to {status}")
        self._status = status

    @property
    def subjobs(self):
        """
        A subjob cannot have subjobs. Always return empty list.
        """
        return []

    @property
    def job_dict(self):
        """
        Returns:
            dict: A JSON serialisable representation of the `SubJob`. `Path` objects are converted to
            `string` via ``Path.as_posix()``. Since Subjobs inherit most of the parent job's config
            we only output the input files and arguments that are specific to this subjob and no other details.
        """
        job_dict = {}
        job_dict["id"] = self.id
        job_dict["input_files"] = self.input_files
        job_dict["args"] = self.args
        return job_dict

    def __getattr__(self, attribute):
        """
        Since a SubJob uses attributes from the parent Job, everything simply accesses the Job attributes
        unless otherwise specified.
        """
        return getattr(self.parent, attribute)

    def __repr__(self):
        """
        """
        return f"SubJob({self.name})"


class Backend(ABC):
    """
    Abstract base class for a valid backend.
    Classes derived from this will implement their own submission of basf2 jobs
    to whatever backend they describe.
    Some common methods/attributes go into this base class.

    For backend_args the priority from lowest to highest is:

    backend.default_backend_args -> backend.backend_args -> job.backend_args
    """
    #: Default submission script name
    submit_script = "submit.sh"
    #: Default exit code file name
    exit_code_file = "__BACKEND_CMD_EXIT_STATUS__"
    #: Default backend_args
    default_backend_args = {}

    def __init__(self, *, backend_args=None):
        """
        """
        if backend_args is None:
            backend_args = {}
        #: The backend args that will be applied to jobs unless the job specifies them itself
        self.backend_args = {**self.default_backend_args, **backend_args}

    @abstractmethod
    def submit(self, job):
        """
        Base method for submitting collection jobs to the backend type. This MUST be
        implemented for a correctly written backend class deriving from Backend().
        """

    @staticmethod
    def _add_setup(job, batch_file):
        """
        Adds setup lines to the shell script file.
        """
        for line in job.setup_cmds:
            print(line, file=batch_file)

    def _add_wrapper_script_setup(self, job, batch_file):
        """
        Adds lines to the submitted script that help with job monitoring/setup. Mostly here so that we can insert
        `trap` statements for Ctrl-C situations.
        """
        start_wrapper = f"""# ---
# trap ctrl-c and call ctrl_c()
trap '(ctrl_c 130)' SIGINT
trap '(ctrl_c 143)' SIGTERM

function write_exit_code() {{
  echo "Writing $1 to exit status file"
  echo "$1" > {self.exit_code_file}
  exit $1
}}

function ctrl_c() {{
  trap '' SIGINT SIGTERM
  echo "** Trapped Ctrl-C **"
  echo "$1" > {self.exit_code_file}
  exit $1
}}
# ---"""
        print(start_wrapper, file=batch_file)

    def _add_wrapper_script_teardown(self, job, batch_file):
        """
        Adds lines to the submitted script that help with job monitoring/teardown. Mostly here so that we can insert
        an exit code of the job cmd being written out to a file. Which means that we can know if the command was
        successful or not even if the backend server/monitoring database purges the data about our job i.e. If PBS
        removes job information too quickly we may never know if a job succeeded or failed without some kind of exit
        file.
        """
        end_wrapper = """# ---
write_exit_code $?"""
        print(end_wrapper, file=batch_file)

    @classmethod
    def _create_parent_job_result(cls, parent):
        """
        We want to be able to call `ready()` on the top level `Job.result`. So this method needs to exist
        so that a Job.result object actually exists. It will be mostly empty and simply updates subjob
        statuses and allows the use of ready().
        """
        raise NotImplementedError

    def get_submit_script_path(self, job):
        """
        Construct the Path object of the bash script file that we will submit. It will contain
        the actual job command, wrapper commands, setup commands, and any batch directives
        """
        return Path(job.working_dir, self.submit_script)


class Result():
    """
    Base class for Result objects. A Result is created for each `Job` (or `Job.SubJob`) object
    submitted to a backend. It provides a way to query a job's status to find out if it's ready.
    """

    def __init__(self, job):
        """
        Pass in the job object to allow the result to access the job's properties and do post-processing.
        """
        #: Job object for result
        self.job = job
        #: Quicker way to know if it's ready once it has already been found. Saves a lot of calls to batch system commands.
        self._is_ready = False
        #: After our first attempt to view the exit code file once the job is 'finished',
        #  how long should we wait for it to exist before timing out?
        self.time_to_wait_for_exit_code_file = timedelta(minutes=5)
        #: Time we started waiting for the exit code file to appear
        self.exit_code_file_initial_time = None

    def ready(self):
        """
        Returns whether or not this job result is known to be ready. Doesn't actually change the job status. Just changes
        the 'readiness' based on the known job status.
        """
        B2DEBUG(29, f"Calling {self.job}.result.ready()")
        if self._is_ready:
            return True
        elif self.job.status in self.job.exit_statuses:
            self._is_ready = True
            return True
        else:
            return False

    def update_status(self):
        """
        Update the job's (and subjobs') status so that `Result.ready` will return the up to date status. This call will have to
        actually look up the job's status from some database/exit code file.
        """
        raise NotImplementedError

    def get_exit_code_from_file(self):
        """
        Read the exit code file to discover the exit status of the job command. Useful falback if the job is no longer
        known to the job database (batch system purged it for example). Since some backends may take time to download
        the output files of the job back to the working directory we use a time limit on how long to wait.
        """
        if not self.exit_code_file_initial_time:
            self.exit_code_file_initial_time = datetime.now()
        exit_code_path = Path(self.job.working_dir, Backend.exit_code_file)
        with open(exit_code_path, "r") as f:
            exit_code = int(f.read().strip())
            B2DEBUG(29, f"Exit code from file for {self.job} was {exit_code}")
            return exit_code


class Local(Backend):
    """
    Backend for local processes i.e. on the same machine but in a subprocess.

    Note that you should call the self.join() method to close the pool and wait for any
    running processes to finish before exiting the process. Once you've called join you will have to set up a new
    instance of this backend to create a new pool. If you don't call `Local.join` or don't create a join yourself
    somewhere, then the main python process might end before your pool is done.

    Keyword Arguments:
        max_processes (int): Integer that specifies the size of the process pool that spawns the subjobs, default=1.
            It's the maximium simultaneous subjobs.
            Try not to specify a large number or a number larger than the number of cores.
            It won't crash the program but it will slow down and negatively impact performance.
    """

    def __init__(self, *, backend_args=None, max_processes=1):
        """
        """
        super().__init__(backend_args=backend_args)
        #: The actual ``Pool`` object of this instance of the Backend.
        self.pool = None
        #: The size of the multiprocessing process pool.
        self.max_processes = max_processes

    class LocalResult(Result):
        """
        Result class to help monitor status of jobs submitted by Local backend.
        """

        def __init__(self, job, result):
            """
            Pass in the job object and the multiprocessing result to allow the result to do monitoring and perform
            post processing of the job.
            """
            super().__init__(job)
            #: The underlying result from the backend
            self.result = result

        def _update_result_status(self):
            if self.result.ready() and (self.job.status not in self.job.exit_statuses):
                return_code = self.result.get()
                if return_code:
                    self.job.status = "failed"
                else:
                    self.job.status = "completed"

        def update_status(self):
            """
            Update the job's (or subjobs') status by calling the result object.
            """
            B2DEBUG(29, f"Calling {self.job}.result.update_status()")
            if self.job.subjobs:
                for subjob in self.job.subjobs.values():
                    subjob.result._update_result_status()
            else:
                self._update_result_status()

    def join(self):
        """
        Closes and joins the Pool, letting you wait for all results currently
        still processing.
        """
        B2INFO("Joining Process Pool, waiting for results to finish...")
        self.pool.close()
        self.pool.join()
        B2INFO("Process Pool joined.")

    @property
    def max_processes(self):
        """
        Getter for max_processes
        """
        return self._max_processes

    @max_processes.setter
    def max_processes(self, value):
        """
        Setter for max_processes, we also check for a previous Pool(), wait for it to join
        and then create a new one with the new value of max_processes
        """
        #: Internal attribute of max_processes
        self._max_processes = value
        if self.pool:
            B2INFO(f"New max_processes requested. But a pool already exists.")
            self.join()
        B2INFO(f"Starting up new Pool with {self.max_processes} processes")
        self.pool = mp.Pool(processes=self.max_processes)

    @method_dispatch
    def submit(self, job):
        """
        """
        raise NotImplementedError(("This is an abstract submit(job) method that shouldn't have been called. "
                                   "Did you submit a (Sub)Job?"))

    @submit.register(SubJob)
    def _(self, job):
        """
        Submission of a `SubJob` for the Local backend
        """
        # Make sure the output directory of the job is created
        job.output_dir.mkdir(parents=True, exist_ok=True)
        # Make sure the working directory of the job is created
        job.working_dir.mkdir(parents=True, exist_ok=True)
        job.copy_input_sandbox_files_to_working_dir()
        job.dump_input_data()
        # Get the path to the bash script we run
        script_path = self.get_submit_script_path(job)
        with open(script_path, mode="w") as batch_file:
            print("#!/bin/bash", file=batch_file)
            self._add_wrapper_script_setup(job, batch_file)
            self._add_setup(job, batch_file)
            print(job.full_command, file=batch_file)
            self._add_wrapper_script_teardown(job, batch_file)
        B2INFO(f"Submitting {job}")
        job.result = Local.LocalResult(job,
                                       self.pool.apply_async(self.run_job,
                                                             (job.name,
                                                              job.working_dir,
                                                              job.output_dir,
                                                              script_path)
                                                             )
                                       )
        job.status = "submitted"
        B2INFO(f"{job} submitted")

    @submit.register(Job)
    def _(self, job):
        """
        Submission of a `Job` for the Local backend
        """
        # Make sure the output directory of the job is created
        job.output_dir.mkdir(parents=True, exist_ok=True)
        # Make sure the working directory of the job is created
        job.working_dir.mkdir(parents=True, exist_ok=True)
        # Check if we have any valid input files
        job.check_input_data_files()

        if not job.splitter:
            # Get all of the requested files for the input sandbox and copy them to the working directory
            job.copy_input_sandbox_files_to_working_dir()
            job.dump_input_data()
            # Get the path to the bash script we run
            script_path = self.get_submit_script_path(job)
            with open(script_path, mode="w") as batch_file:
                print("#!/bin/bash", file=batch_file)
                self._add_wrapper_script_setup(job, batch_file)
                self._add_setup(job, batch_file)
                print(job.full_command, file=batch_file)
                self._add_wrapper_script_teardown(job, batch_file)
            B2INFO(f"Submitting {job}")
            job.result = Local.LocalResult(job,
                                           self.pool.apply_async(self.run_job,
                                                                 (job.name,
                                                                  job.working_dir,
                                                                  job.output_dir,
                                                                  script_path)
                                                                 )
                                           )
            B2INFO(f"{job} submitted")
        else:
            # Create subjobs according to the splitter's logic
            job.splitter.create_subjobs(job)
            # Submit the subjobs
            self.submit(list(job.subjobs.values()))
            # After submitting subjobs, make a Job.result for the parent Job object, used to call ready() on
            self._create_parent_job_result(job)

    @submit.register(list)
    def _(self, jobs):
        """
        Submit method of Local() that takes a list of jobs instead of just one and submits each one.
        """
        # Submit the jobs
        for job in jobs:
            self.submit(job)
        B2INFO("All requested jobs submitted.")

    @staticmethod
    def run_job(name, working_dir, output_dir, script):
        """
        The function that is used by multiprocessing.Pool.apply_async during process creation. This runs a
        shell command in a subprocess and captures the stdout and stderr of the subprocess to files.
        """
        B2INFO(f"Starting Sub-process: {name}")
        from subprocess import Popen
        stdout_file_path = Path(working_dir, _STDOUT_FILE)
        stderr_file_path = Path(working_dir, _STDERR_FILE)
        # Create unix command to redirect stdour and stderr
        B2INFO(f"stdout/err for subprocess {name} visible at:\n\t{stdout_file_path}\n\t{stderr_file_path}")
        with open(stdout_file_path, mode="w", buffering=1) as f_out, \
                open(stderr_file_path, mode="w", buffering=1) as f_err:
            with Popen(["/bin/bash", script.as_posix()],
                       stdout=f_out,
                       stderr=f_err,
                       bufsize=1,
                       universal_newlines=True,
                       cwd=working_dir,
                       env={}) as p:
                # We block here and wait so that the return code will be set.
                p.wait()
        B2INFO(f"Subprocess {name} finished.")
        return p.returncode

    @classmethod
    def _create_parent_job_result(cls, parent):
        parent.result = cls.LocalResult(parent, None)


class Batch(Backend):
    """
    Abstract Base backend for submitting to a local batch system. Batch system specific commands should be implemented
    in a derived class. Do not use this class directly!
    """
    #: Shell command to submit a script, should be implemented in the derived class
    submission_cmds = []
    #: Default global limit on the total number of submitted/running jobs that the user can have.
    #  This limit will not affect the **total** number of jobs that are eventually submitted.
    #  But the jobs won't actually be submitted until this limit can be respected i.e. until the number
    #  of total jobs in the Batch system goes down.
    #  Since we actually submit in chunks of N jobs, before checking this limit value again, this value
    #  needs to be a little lower than the real batch system limit.
    #  Otherwise you could accidentally go over during the N job submission if other processes
    #  are checking and submitting concurrently. This is quite common for the first submission of jobs from
    #  parallel calibrations.
    #
    #  Note that if there are other jobs already submitted for your account, then these will count towards
    #  this limit.
    default_global_job_limit = 1000
    #: Default time betweeon re-checking if the active jobs is below the global job limit.
    default_sleep_between_submission_checks = 30

    def __init__(self, *, backend_args=None):
        """
        Init method for Batch Backend. Does some basic default setup.
        """
        super().__init__(backend_args=backend_args)
        #: The active job limit. This is 'global' because we want to prevent us accidentally submitting
        # too many jobs from all current and previous submission scripts.
        self.global_job_limit = self.default_global_job_limit
        #: Seconds we wait before checking if we can submit a list of jobs. Only relevant once we hit the global
        # limit of active jobs, which is a lot usually.
        self.sleep_between_submission_checks = self.default_sleep_between_submission_checks

    def _add_batch_directives(self, job, file):
        """
        Should be implemented in a derived class to write a batch submission script to the job.working_dir.
        You should think about where the stdout/err should go, and set the queue name.
        """
        raise NotImplementedError(("Need to implement a _add_batch_directives(self, job, file) "
                                   f"method in {self.__class__.__name__} backend."))

    def _make_submit_file(self, job, submit_file_path):
        """
        Useful for the HTCondor backend where a submit is needed instead of batch
        directives pasted directly into the submission script. It should be overwritten
        if needed.
        """

    @classmethod
    @abstractmethod
    def _submit_to_batch(cls, cmd):
        """
        Do the actual batch submission command and collect the output to find out the job id for later monitoring.
        """

    def can_submit(self, *args, **kwargs):
        """
        Should be implemented in a derived class to check that submitting the next job(s) shouldn't fail.
        This is initially meant to make sure that we don't go over the global limits of jobs (submitted + running).

        Returns:
            bool: If the job submission can continue based on the current situation.
        """
        return True

    @method_dispatch
    def submit(self, job, check_can_submit=True, jobs_per_check=100):
        """
        """
        raise NotImplementedError(("This is an abstract submit(job) method that shouldn't have been called. "
                                   "Did you submit a (Sub)Job?"))

    @submit.register(SubJob)
    def _(self, job, check_can_submit=True, jobs_per_check=100):
        """
        Submit method of Batch backend for a `SubJob`. Should take `SubJob` object, create needed directories,
        create batch script, and send it off with the batch submission command.
        It should apply the correct options (default and user requested).

        Should set a Result object as an attribute of the job.
        """
        # Make sure the output directory of the job is created, commented out due to permission issues
        # job.output_dir.mkdir(parents=True, exist_ok=True)
        # Make sure the working directory of the job is created
        job.working_dir.mkdir(parents=True, exist_ok=True)
        job.copy_input_sandbox_files_to_working_dir()
        job.dump_input_data()
        # Make submission file if needed
        batch_submit_script_path = self.get_batch_submit_script_path(job)
        self._make_submit_file(job, batch_submit_script_path)
        # Get the bash file we will actually run, might be the same file
        script_path = self.get_submit_script_path(job)
        # Construct the batch submission script (with directives if that is supported)
        with open(script_path, mode="w") as batch_file:
            self._add_batch_directives(job, batch_file)
            self._add_wrapper_script_setup(job, batch_file)
            self._add_setup(job, batch_file)
            print(job.full_command, file=batch_file)
            self._add_wrapper_script_teardown(job, batch_file)
        os.chmod(script_path, 0o755)
        B2INFO(f"Submitting {job}")
        # Do the actual batch submission
        cmd = self._create_cmd(batch_submit_script_path)
        output = self._submit_to_batch(cmd)
        self._create_job_result(job, output)
        job.status = "submitted"
        B2INFO(f"{job} submitted")

    @submit.register(Job)
    def _(self, job, check_can_submit=True, jobs_per_check=100):
        """
        Submit method of Batch backend. Should take job object, create needed directories, create batch script,
        and send it off with the batch submission command, applying the correct options (default and user requested.)

        Should set a Result object as an attribute of the job.
        """
        # Make sure the output directory of the job is created, commented out due to permissions issue
        # job.output_dir.mkdir(parents=True, exist_ok=True)
        # Make sure the working directory of the job is created
        job.working_dir.mkdir(parents=True, exist_ok=True)
        # Check if we have any valid input files
        job.check_input_data_files()
        # Add any required backend args that are missing (I'm a bit hesitant to actually merge with job.backend_args)
        # just in case you want to resubmit the same job with different backend settings later.
        job_backend_args = {**self.backend_args, **job.backend_args}

        # If there's no splitter then we just submit the Job with no SubJobs
        if not job.splitter:
            # Get all of the requested files for the input sandbox and copy them to the working directory
            job.copy_input_sandbox_files_to_working_dir()
            job.dump_input_data()
            # Make submission file if needed
            batch_submit_script_path = self.get_batch_submit_script_path(job)
            self._make_submit_file(job, batch_submit_script_path)
            # Get the bash file we will actually run
            script_path = self.get_submit_script_path(job)
            # Construct the batch submission script (with directives if that is supported)
            with open(script_path, mode="w") as batch_file:
                self._add_batch_directives(job, batch_file)
                self._add_wrapper_script_setup(job, batch_file)
                self._add_setup(job, batch_file)
                print(job.full_command, file=batch_file)
                self._add_wrapper_script_teardown(job, batch_file)
            os.chmod(script_path, 0o755)
            B2INFO(f"Submitting {job}")
            # Do the actual batch submission
            cmd = self._create_cmd(batch_submit_script_path)
            output = self._submit_to_batch(cmd)
            self._create_job_result(job, output)
            job.status = "submitted"
            B2INFO(f"{job} submitted")
        else:
            # Create subjobs according to the splitter's logic
            job.splitter.create_subjobs(job)
            # Submit the subjobs
            self.submit(list(job.subjobs.values()))
            # After submitting subjobs, make a Job.result for the parent Job object, used to call ready() on
            self._create_parent_job_result(job)

    @submit.register(list)
    def _(self, jobs, check_can_submit=True, jobs_per_check=100):
        """
        Submit method of Batch Backend that takes a list of jobs instead of just one and submits each one.
        """
        B2INFO(f"Submitting a list of {len(jobs)} jobs to a Batch backend")
        # Technically this could be a list of Jobs or SubJobs. And if it is a list of Jobs then it might not
        # be necessary to check if we can submit right now. We could do it later during the submission of the
        # SubJob list. However in the interest of simpler code we just do the check here, and re-check again
        # if a SubJob list comes through this function. Slightly inefficient, but much simpler logic.

        # The first thing to do is make sure that we are iterating through the jobs list in chunks that are
        # equal to or smaller than the gloabl limit. Otherwise nothing will ever submit.

        if jobs_per_check > self.global_job_limit:
            B2INFO((f"jobs_per_check (={jobs_per_check}) but this is higher than the global job "
                    f"limit for this backend (={self.global_job_limit}). Will instead use the "
                    " value of the global job limit."))
            jobs_per_check = self.global_job_limit

        # We group the jobs list into chunks of length jobs_per_check
        for jobs_to_submit in grouper(jobs_per_check, jobs):
            # Wait until we are allowed to submit
            while not self.can_submit(njobs=len(jobs_to_submit)):
                B2INFO(f"Too many jobs are currently in the batch system globally. Waiting until submission can continue...")
                time.sleep(self.sleep_between_submission_checks)
            else:
                # We loop here since we have already checked if the number of jobs is low enough, we don't want to hit this
                # function again unless one of the jobs has subjobs.
                B2INFO(f"Submitting the next {len(jobs_to_submit)} jobs...")
                for job in jobs_to_submit:
                    self.submit(job, check_can_submit, jobs_per_check)
        B2INFO(f"All {len(jobs)} requested jobs submitted")

    def get_batch_submit_script_path(self, job):
        """
        Construct the Path object of the script file that we will submit using the batch command.
        For most batch backends this is the same script as the bash script we submit.
        But for some they require a separate submission file that describes the job.
        To implement that you can implement this function in the Backend class.
        """
        return Path(job.working_dir, self.submit_script)

    @classmethod
    @abstractmethod
    def _create_job_result(cls, job, batch_output):
        """
        """

    @abstractmethod
    def _create_cmd(self, job):
        """
        """


class PBS(Batch):
    """
    Backend for submitting calibration processes to a qsub batch system.
    """
    #: Working directory directive
    cmd_wkdir = "#PBS -d"
    #: stdout file directive
    cmd_stdout = "#PBS -o"
    #: stderr file directive
    cmd_stderr = "#PBS -e"
    #: Queue directive
    cmd_queue = "#PBS -q"
    #: Job name directive
    cmd_name = "#PBS -N"
    #: Shell command to submit a script
    submission_cmds = ["qsub"]
    #: Default global limit on the number of jobs to have in the system at any one time.
    default_global_job_limit = 5000
    #: Default backend_args for PBS
    default_backend_args = {"queue": "short"}

    def __init__(self, *, backend_args=None):
        super().__init__(backend_args=backend_args)

    def _add_batch_directives(self, job, batch_file):
        """
        Add PBS directives to submitted script.
        """
        job_backend_args = {**self.backend_args, **job.backend_args}
        batch_queue = job_backend_args["queue"]
        print("#!/bin/bash", file=batch_file)
        print("# --- Start PBS ---", file=batch_file)
        print(" ".join([PBS.cmd_queue, batch_queue]), file=batch_file)
        print(" ".join([PBS.cmd_name, job.name]), file=batch_file)
        print(" ".join([PBS.cmd_wkdir, job.working_dir.as_posix()]), file=batch_file)
        print(" ".join([PBS.cmd_stdout, Path(job.working_dir, _STDOUT_FILE).as_posix()]), file=batch_file)
        print(" ".join([PBS.cmd_stderr, Path(job.working_dir, _STDERR_FILE).as_posix()]), file=batch_file)
        print("# --- End PBS ---", file=batch_file)

    @classmethod
    def _create_job_result(cls, job, batch_output):
        """
        """
        job_id = batch_output.replace("\n", "")
        B2INFO(f"Job ID of {job} recorded as: {job_id}")
        job.result = cls.PBSResult(job, job_id)

    def _create_cmd(self, script_path):
        """
        """
        submission_cmd = self.submission_cmds[:]
        submission_cmd.append(script_path.as_posix())
        return submission_cmd

    @classmethod
    def _submit_to_batch(cls, cmd):
        """
        Do the actual batch submission command and collect the output to find out the job id for later monitoring.
        """
        sub_out = subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True)
        return sub_out

    @classmethod
    def _create_parent_job_result(cls, parent):
        parent.result = cls.PBSResult(parent, None)

    class PBSResult(Result):
        """
        Simple class to help monitor status of jobs submitted by `PBS` Backend.

        You pass in a `Job` object (or `SubJob`) and job id from a qsub command.
        When you call the `ready` method it runs bjobs to see whether or not the job has finished.
        """

        #: PBS statuses mapped to Job statuses
        backend_code_to_status = {"R": "running",
                                  "C": "completed",
                                  "FINISHED": "completed",
                                  "E": "failed",
                                  "H": "submitted",
                                  "Q": "submitted",
                                  "T": "submitted",
                                  "W": "submitted",
                                  "H": "submitted"
                                  }

        def __init__(self, job, job_id):
            """
            Pass in the job object and the job id to allow the result to do monitoring and perform
            post processing of the job.
            """
            super().__init__(job)
            #: job id given by PBS
            self.job_id = job_id

        def update_status(self):
            """
            Update the job's (or subjobs') status by calling qstat.
            """
            B2DEBUG(29, f"Calling {self.job}.result.update_status()")
            # Get all jobs info and re-use it for each status update to minimise tie spent on this updating.
            qstat_output = PBS.qstat()
            if self.job.subjobs:
                for subjob in self.job.subjobs.values():
                    subjob.result._update_result_status(qstat_output)
            else:
                self._update_result_status(qstat_output)

        def _update_result_status(self, qstat_output):
            """
            Parameters:
                    qstat_output (dict): The JSON output of a previous call to qstat which we can re-use to find the
                    status of this job. Obviously you should only be passing a JSON dict that contains the 'Job_Id' and
                    'job_state' information, otherwise it is useless.

            """
            try:
                backend_status = self._get_status_from_output(qstat_output)
            except KeyError:
                # If this happens then maybe the job id wasn't in the qstat_output argument because it finished.
                # Instead of failing immediately we try looking for the exit code file and then fail if it still isn't there.
                B2DEBUG(29, f"Checking of the exit code from file for {self.job}")
                try:
                    exit_code = self.get_exit_code_from_file()
                except FileNotFoundError:
                    waiting_time = datetime.now() - self.exit_code_file_initial_time
                    if self.time_to_wait_for_exit_code_file > waiting_time:
                        B2ERROR(f"Exit code file for {self.job} missing and we can't wait longer. Setting exit code to 1.")
                        exit_code = 1
                    else:
                        B2WARNING(f"Exit code file for {self.job} missing, will wait longer.")
                        return
                if exit_code:
                    backend_status = "E"
                else:
                    backend_status = "C"

            try:
                new_job_status = self.backend_code_to_status[backend_status]
            except KeyError as err:
                raise BackendError(f"Unidentified backend status found for {self.job}: {backend_status}")

            if new_job_status != self.job.status:
                self.job.status = new_job_status

        def _get_status_from_output(self, output):
            for job_info in output["JOBS"]:
                if job_info["Job_Id"] == self.job_id:
                    return job_info["job_state"]
            else:
                raise KeyError

    def can_submit(self, njobs=1):
        """
        Checks the global number of jobs in PBS right now (submitted or running) for this user.
        Returns True if the number is lower that the limit, False if it is higher.

        Parameters:
            njobs (int): The number of jobs that we want to submit before checking again. Lets us check if we
                are sufficiently below the limit in order to (somewhat) safely submit. It is slightly dangerous to
                assume that it is safe to submit too many jobs since there might be other processes also submitting jobs.
                So njobs really shouldn't be abused when you might be getting close to the limit i.e. keep it <=250
                and check again before submitting more.
        """
        B2DEBUG(29, "Calling PBS().can_submit()")
        job_info = self.qstat(username=os.environ["USER"])
        total_jobs = job_info["NJOBS"]
        B2INFO(f"Total jobs active in the PBS system is currently {total_jobs}")
        if (total_jobs + njobs) > self.global_job_limit:
            B2INFO(f"Since the global limit is {self.global_job_limit} we cannot submit {njobs} jobs until some complete.")
            return False
        else:
            B2INFO("There is enough space to submit more jobs.")
            return True

    @classmethod
    def qstat(cls, username="", job_ids=None):
        """
        Simplistic interface to the ``qstat`` command. Lets you request information about all jobs or ones matching the filter
        ['job_id'] or for the username. The result is a JSON dictionary containing come of the useful job attributes returned
        by qstat.

        PBS is kind of annoying as depending on the configuration it can forget about jobs immediately. So the status of a
        finished job is VERY hard to get. There are other commands that are sometimes included that may do a better job.
        This one should work for Melbourne's cloud computing centre.

        Keyword Args:
            username (str): The username of the jobs we are interested in. Only jobs corresponding to the <username>@hostnames
                will be in the output dictionary.
            job_ids (list[str]): List of Job ID strings, each given by qstat during submission. If this argument is given then
                the output of this function will be only information about this jobs. If this argument is not given, then all jobs
                matching the other filters will be returned.

        Returns:
            dict: JSON dictionary of the form (to save you parsing the XML that qstat returns).:

            .. code-block:: python

              {
                "NJOBS": int
                "JOBS":[
                        {
                          <key: value>, ...
                        }, ...
                       ]
              }
        """
        B2DEBUG(29, f"Calling PBS.qstat(username='{username}', job_id={job_ids})")
        if not job_ids:
            job_ids = []
        job_ids = set(job_ids)
        cmd_list = ["qstat", "-x"]
        # We get an XML serialisable summary from qstat. Requires the shell argument.
        cmd = " ".join(cmd_list)
        B2DEBUG(29, f"Calling subprocess with command = '{cmd}'")
        output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True, shell=True)
        jobs_dict = {"NJOBS": 0, "JOBS": []}
        jobs_xml = ET.fromstring(output)

        # For a specific job_id we can be a bit more efficient in XML parsing
        if len(job_ids) == 1:
            job_elem = jobs_xml.find(f"./Job[Job_Id='{list(job_ids)[0]}']")
            if job_elem:
                jobs_dict["JOBS"].append(cls.create_job_record_from_element(job_elem))
                jobs_dict["NJOBS"] = 1
            return jobs_dict

        # Since the username given is not exactly the same as the one that PBS stores (<username>@host)
        # we have to simply loop through rather than using XPATH.
        for job in jobs_xml.iterfind("Job"):
            job_owner = job.find("Job_Owner").text.split("@")[0]
            if username and username != job_owner:
                continue
            job_id = job.find("Job_Id").text
            if job_ids and job_id not in job_ids:
                continue
            jobs_dict["JOBS"].append(cls.create_job_record_from_element(job))
            jobs_dict["NJOBS"] += 1
            # Remove it so that we don't keep checking for it
            if job_id in job_ids:
                job_ids.remove(job_id)
        return jobs_dict

    @staticmethod
    def create_job_record_from_element(job_elem):
        """
        Creates a Job dictionary with various job information from the XML element returned by qstat.

        Parameters:
            job_elem (xml.etree.ElementTree.Element): The XML Element of the Job

        Returns:
            dict: JSON serialisable dictionary of the Job information we are interested in.
        """
        job_dict = {}
        job_dict["Job_Id"] = job_elem.find("Job_Id").text
        job_dict["Job_Name"] = job_elem.find("Job_Name").text
        job_dict["Job_Owner"] = job_elem.find("Job_Owner").text
        job_dict["job_state"] = job_elem.find("job_state").text
        job_dict["queue"] = job_elem.find("queue").text
        return job_dict


class LSF(Batch):
    """
    Backend for submitting calibration processes to a qsub batch system.
    """
    #: Working directory directive
    cmd_wkdir = "#BSUB -cwd"
    #: stdout file directive
    cmd_stdout = "#BSUB -o"
    #: stderr file directive
    cmd_stderr = "#BSUB -e"
    #: Queue directive
    cmd_queue = "#BSUB -q"
    #: Job name directive
    cmd_name = "#BSUB -J"
    #: Shell command to submit a script
    submission_cmds = ["bsub", "-env", "\"none\"", "<"]
    #: Default global limit on the number of jobs to have in the system at any one time.
    default_global_job_limit = 15000
    #: Default backend args for LSF
    default_backend_args = {"queue": "s"}

    def __init__(self, *, backend_args=None):
        super().__init__(backend_args=backend_args)

    def _add_batch_directives(self, job, batch_file):
        """
        Adds LSF BSUB directives for the job to a script.
        """
        job_backend_args = {**self.backend_args, **job.backend_args}  # Merge the two dictionaries, with the job having priority
        batch_queue = job_backend_args["queue"]
        print("#!/bin/bash", file=batch_file)
        print("# --- Start LSF ---", file=batch_file)
        print(" ".join([LSF.cmd_queue, batch_queue]), file=batch_file)
        print(" ".join([LSF.cmd_name, job.name]), file=batch_file)
        print(" ".join([LSF.cmd_wkdir, str(job.working_dir)]), file=batch_file)
        print(" ".join([LSF.cmd_stdout, Path(job.working_dir, _STDOUT_FILE).as_posix()]), file=batch_file)
        print(" ".join([LSF.cmd_stderr, Path(job.working_dir, _STDERR_FILE).as_posix()]), file=batch_file)
        print("# --- End LSF ---", file=batch_file)

    def _create_cmd(self, script_path):
        """
        """
        submission_cmd = self.submission_cmds[:]
        submission_cmd.append(script_path.as_posix())
        submission_cmd = " ".join(submission_cmd)
        return [submission_cmd]

    @classmethod
    def _submit_to_batch(cls, cmd):
        """
        Do the actual batch submission command and collect the output to find out the job id for later monitoring.
        """
        sub_out = subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True, shell=True)
        return sub_out

    class LSFResult(Result):
        """
        Simple class to help monitor status of jobs submitted by LSF Backend.

        You pass in a `Job` object and job id from a bsub command.
        When you call the `ready` method it runs bjobs to see whether or not the job has finished.
        """

        #: LSF statuses mapped to Job statuses
        backend_code_to_status = {"RUN": "running",
                                  "DONE": "completed",
                                  "FINISHED": "completed",
                                  "EXIT": "failed",
                                  "PEND": "submitted"
                                  }

        def __init__(self, job, job_id):
            """
            Pass in the job object and the job id to allow the result to do monitoring and perform
            post processing of the job.
            """
            super().__init__(job)
            #: job id given by LSF
            self.job_id = job_id

        def update_status(self):
            """
            Update the job's (or subjobs') status by calling bjobs.
            """
            B2DEBUG(29, f"Calling {self.job.name}.result.update_status()")
            # Get all jobs info and re-use it for each status update to minimise tie spent on this updating.
            bjobs_output = LSF.bjobs(output_fields=["stat", "id"])
            if self.job.subjobs:
                for subjob in self.job.subjobs.values():
                    subjob.result._update_result_status(bjobs_output)
            else:
                self._update_result_status(bjobs_output)

        def _update_result_status(self, bjobs_output):
            """
            Parameters:
                bjobs_output (dict): The JSON output of a previous call to bjobs which we can re-use to find the
                    status of this job. Obviously you should only be passing a JSON dict that contains the 'stat' and
                    'id' information, otherwise it is useless.

            """
            try:
                backend_status = self._get_status_from_output(bjobs_output)
            except KeyError:
                # If this happens then maybe the job id wasn't in the bjobs_output argument because it finished.
                # Instead of failing immediately we try re-running bjobs here explicitly and then fail if it still isn't there.
                bjobs_output = LSF.bjobs(output_fields=["stat", "id"], job_id=str(self.job_id))
                try:
                    backend_status = self._get_status_from_output(bjobs_output)
                except KeyError:
                    # If this happened, maybe we're looking at an old finished job. We could fall back to bhist, but it's
                    # slow and terrible. Instead let's try looking for the exit code file.
                    try:
                        exit_code = self.get_exit_code_from_file()
                    except FileNotFoundError:
                        waiting_time = datetime.now() - self.exit_code_file_initial_time
                        if self.time_to_wait_for_exit_code_file > waiting_time:
                            B2ERROR(f"Exit code file for {self.job} missing and we can't wait longer. Setting exit code to 1.")
                            exit_code = 1
                        else:
                            B2WARNING(f"Exit code file for {self.job} missing, will wait longer.")
                            return
                    if exit_code:
                        backend_status = "EXIT"
                    else:
                        backend_status = "FINISHED"
            try:
                new_job_status = self.backend_code_to_status[backend_status]
            except KeyError as err:
                raise BackendError(f"Unidentified backend status found for {self.job}: {backend_status}")

            if new_job_status != self.job.status:
                self.job.status = new_job_status

        def _get_status_from_output(self, output):
            if output["JOBS"] and "ERROR" in output["JOBS"][0]:
                if output["JOBS"][0]["ERROR"] == f"Job <{self.job_id}> is not found":
                    raise KeyError(f"No job record in the 'output' argument had the 'JOBID'=={self.job_id}")
                else:
                    raise BackendError(f"Unidentified Error during status check for {self.job}: {output}")
            else:
                for job_info in output["JOBS"]:
                    if job_info["JOBID"] == self.job_id:
                        return job_info["STAT"]
                else:
                    raise KeyError(f"No job record in the 'output' argument had the 'JOBID'=={self.job_id}")

    @classmethod
    def _create_parent_job_result(cls, parent):
        parent.result = cls.LSFResult(parent, None)

    @classmethod
    def _create_job_result(cls, job, batch_output):
        """
        """
        m = re.search(r"Job <(\d+)>", str(batch_output))
        if m:
            job_id = m.group(1)
        else:
            raise BackendError(f"Failed to get the batch job ID of {job}. LSF output was:\n{batch_output}")

        B2INFO(f"Job ID of {job} recorded as: {job_id}")
        job.result = cls.LSFResult(job, job_id)

    def can_submit(self, njobs=1):
        """
        Checks the global number of jobs in LSF right now (submitted or running) for this user.
        Returns True if the number is lower that the limit, False if it is higher.

        Parameters:
            njobs (int): The number of jobs that we want to submit before checking again. Lets us check if we
                are sufficiently below the limit in order to (somewhat) safely submit. It is slightly dangerous to
                assume that it is safe to submit too many jobs since there might be other processes also submitting jobs.
                So njobs really shouldn't be abused when you might be getting close to the limit i.e. keep it <=250
                and check again before submitting more.
        """
        B2DEBUG(29, "Calling LSF().can_submit()")
        job_info = self.bjobs(output_fields=["stat"])
        total_jobs = job_info["NJOBS"]
        B2INFO(f"Total jobs active in the LSF system is currently {total_jobs}")
        if (total_jobs + njobs) > self.global_job_limit:
            B2INFO(f"Since the global limit is {self.global_job_limit} we cannot submit {njobs} jobs until some complete.")
            return False
        else:
            B2INFO("There is enough space to submit more jobs.")
            return True

    @classmethod
    def bjobs(cls, output_fields=None, job_id="", username="", queue=""):
        """
        Simplistic interface to the `bjobs` command. lets you request information about all jobs matching the filters
        'job_id', 'username', and 'queue'. The result is the JSON dictionary returned by output of the ``-json`` bjobs option.

        Parameters:
            output_fields (list[str]): A list of bjobs -o fields that you would like information about e.g. ['stat', 'name', 'id']
            job_id (str): String representation of the Job ID given by bsub during submission If this argument is given then
                the output of this function will be only information about this job. If this argument is not given, then all jobs
                matching the other filters will be returned.
            username (str): By default bjobs (and this function) return information about only the current user's jobs. By giving
                a username you can access the job information of a specific user's jobs. By giving ``username='all'`` you will
                receive job information from all known user jobs matching the other filters.
            queue (str): Set this argument to receive job information about jobs that are in the given queue and no other.

        Returns:
            dict: JSON dictionary of the form:

            .. code-block:: python

              {
                "NJOBS":<njobs returned by command>,
                "JOBS":[
                        {
                          <output field: value>, ...
                        }, ...
                       ]
              }
        """
        B2DEBUG(29, f"Calling LSF.bjobs(output_fields={output_fields}, job_id={job_id}, username={username}, queue={queue})")
        # We must always return at least one output field when using JSON and -o options. So we choose the job id
        if not output_fields:
            output_fields = ["id"]
        # Output fields should be space separated but in a string.
        field_list_cmd = "\""
        field_list_cmd += " ".join(output_fields)
        field_list_cmd += "\""
        cmd_list = ["bjobs", "-o", field_list_cmd]
        # If the queue name is set then we add to the command options
        if queue:
            cmd_list.extend(["-q", queue])
        # If the username is set then we add to the command options
        if username:
            cmd_list.extend(["-u", username])
        # Can now add the json option before the final positional argument (if used)
        cmd_list.append("-json")
        # If the job id is set then we add to the end of the command
        if job_id:
            cmd_list.append(job_id)
        # We get a JSON serialisable summary from bjobs. Requires the shell argument.
        cmd = " ".join(cmd_list)
        B2DEBUG(29, f"Calling subprocess with command = '{cmd}'")
        output = decode_json_string(subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True, shell=True))
        output["NJOBS"] = output["JOBS"]
        output["JOBS"] = output["RECORDS"]
        del output["RECORDS"]
        del output["COMMAND"]
        return output

    @classmethod
    def bqueues(cls, output_fields=None, queues=None):
        """
        Simplistic interface to the `bqueues` command. lets you request information about all queues matching the filters.
        The result is the JSON dictionary returned by output of the ``-json`` bqueues option.

        Parameters:
            output_fields (list[str]): A list of bqueues -o fields that you would like information about
                e.g. the default is ['queue_name' 'status' 'max' 'njobs' 'pend' 'run']
            queues (list[str]): Set this argument to receive information about only the queues that are requested and no others.
                By default you will receive information about all queues.

        Returns:
            dict: JSON dictionary of the form:

            .. code-block:: python

              {
                "COMMAND":"bqueues",
                "QUEUES":46,
                "RECORDS":[
                  {
                    "QUEUE_NAME":"b2_beast",
                    "STATUS":"Open:Active",
                    "MAX":"200",
                    "NJOBS":"0",
                    "PEND":"0",
                    "RUN":"0"
                  }, ...
              }
        """
        B2DEBUG(29, f"Calling LSF.bqueues(output_fields={output_fields}, queues={queues})")
        # We must always return at least one output field when using JSON and -o options. So we choose the job id
        if not output_fields:
            output_fields = ["queue_name", "status", "max", "njobs", "pend", "run"]
        # Output fields should be space separated but in a string.
        field_list_cmd = "\""
        field_list_cmd += " ".join(output_fields)
        field_list_cmd += "\""
        cmd_list = ["bqueues", "-o", field_list_cmd]
        # Can now add the json option before the final positional argument (if used)
        cmd_list.append("-json")
        # If the queue name is set then we add to the end of the command
        if queues:
            cmd_list.extend(queues)
        # We get a JSON serialisable summary from bjobs. Requires the shell argument.
        cmd = " ".join(cmd_list)
        B2DEBUG(29, f"Calling subprocess with command = '{cmd}'")
        output = subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True, shell=True)
        return decode_json_string(output)


class HTCondor(Batch):
    """
    Backend for submitting calibration processes to a HTCondor batch system.
    """
    #: HTCondor batch script (different to the wrapper script of `Backend.submit_script`)
    batch_submit_script = "submit.sub"
    #: Batch submission commands for HTCondor
    submission_cmds = ["condor_submit", "-terse"]
    #: Default global limit on the number of jobs to have in the system at any one time.
    default_global_job_limit = 10000
    #: Default backend args for HTCondor
    default_backend_args = {
                            "universe": "vanilla",
                            "getenv": "false",
                            "request_memory": "4 GB",  # We set the default requested memory to 4 GB to maintain parity with KEKCC
                            "path_prefix": "",  # Path prefix for file path
                            "extra_lines": []  # These should be other HTCondor submit script lines like 'request_cpus = 2'
                           }
    #: Default ClassAd attributes to return from commands like condor_q
    default_class_ads = ["GlobalJobId", "JobStatus", "Owner"]

    def _make_submit_file(self, job, submit_file_path):
        """
        Fill HTCondor submission file.
        """
        # Find all files/directories in the working directory to copy on the worker node

        files_to_transfer = [i.as_posix() for i in job.working_dir.iterdir()]

        job_backend_args = {**self.backend_args, **job.backend_args}  # Merge the two dictionaries, with the job having priority

        with open(submit_file_path, "w") as submit_file:
            print(f'executable = {self.get_submit_script_path(job)}', file=submit_file)
            print(f'log = {Path(job.output_dir, "htcondor.log").as_posix()}', file=submit_file)
            print(f'output = {Path(job.working_dir, _STDOUT_FILE).as_posix()}', file=submit_file)
            print(f'error = {Path(job.working_dir, _STDERR_FILE).as_posix()}', file=submit_file)
            print(f'transfer_input_files = ', ','.join(files_to_transfer), file=submit_file)
            print(f'universe = {job_backend_args["universe"]}', file=submit_file)
            print(f'getenv = {job_backend_args["getenv"]}', file=submit_file)
            print(f'request_memory = {job_backend_args["request_memory"]}', file=submit_file)
            print('should_transfer_files = Yes', file=submit_file)
            print('when_to_transfer_output = ON_EXIT', file=submit_file)
            # Any other lines in the backend args that we don't deal with explicitly but maybe someone wants to insert something
            for line in job_backend_args["extra_lines"]:
                print(line, file=submit_file)
            print('queue', file=submit_file)

    def _add_batch_directives(self, job, batch_file):
        """
        For HTCondor leave empty as the directives are already included in the submit file.
        """
        print('#!/bin/bash', file=batch_file)

    def _create_cmd(self, script_path):
        """
        """
        submission_cmd = self.submission_cmds[:]
        submission_cmd.append(script_path.as_posix())
        return submission_cmd

    def get_batch_submit_script_path(self, job):
        """
        Construct the Path object of the .sub file that we will use to describe the job.
        """
        return Path(job.working_dir, self.batch_submit_script)

    @classmethod
    def _submit_to_batch(cls, cmd):
        """
        Do the actual batch submission command and collect the output to find out the job id for later monitoring.
        """
        job_dir = Path(cmd[-1]).parent.as_posix()
        sub_out = ""
        try:
            sub_out = subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True, cwd=job_dir)
        except subprocess.CalledProcessError as e:
            B2ERROR(f"Error during condor_submit: {str(e)}")
            raise e
        return sub_out.split()[0]

    class HTCondorResult(Result):
        """
        Simple class to help monitor status of jobs submitted by HTCondor Backend.

        You pass in a `Job` object and job id from a condor_submit command.
        When you call the `ready` method it runs condor_q and, if needed, ``condor_history``
        to see whether or not the job has finished.
        """

        #: HTCondor statuses mapped to Job statuses
        backend_code_to_status = {0: "submitted",
                                  1: "submitted",
                                  2: "running",
                                  3: "failed",
                                  4: "completed",
                                  5: "submitted",
                                  6: "failed"
                                  }

        def __init__(self, job, job_id):
            """
            Pass in the job object and the job id to allow the result to do monitoring and perform
            post processing of the job.
            """
            super().__init__(job)
            #: job id given by HTCondor
            self.job_id = job_id

        def update_status(self):
            """
            Update the job's (or subjobs') status by calling condor_q.
            """
            B2DEBUG(29, f"Calling {self.job.name}.result.update_status()")
            # Get all jobs info and re-use it for each status update to minimise tie spent on this updating.
            condor_q_output = HTCondor.condor_q()
            if self.job.subjobs:
                for subjob in self.job.subjobs.values():
                    subjob.result._update_result_status(condor_q_output)
            else:
                self._update_result_status(condor_q_output)

        def _update_result_status(self, condor_q_output):
            """
            In order to be slightly more efficient we pass in a previous call to condor_q to see if it can work.
            If it is there we update the job's status. If not we are forced to start calling condor_q and, if needed,
            ``condor_history``, etc.

            Parameters:
                condor_q_output (dict): The JSON output of a previous call to `HTCondor.condor_q` which we can re-use to find the
                    status of this job if it was active when that command ran.
            """
            B2DEBUG(29, f"Calling {self.job}.result._update_result_status()")
            jobs_info = []
            for job_record in condor_q_output["JOBS"]:
                job_id = job_record["GlobalJobId"].split("#")[1]
                if job_id == self.job_id:
                    B2DEBUG(29, f"Found {self.job_id} in condor_q_output.")
                    jobs_info.append(job_record)

            # Let's look for the exit code file where we expect it
            if not jobs_info:
                try:
                    exit_code = self.get_exit_code_from_file()
                except FileNotFoundError:
                    waiting_time = datetime.now() - self.exit_code_file_initial_time
                    if self.time_to_wait_for_exit_code_file > waiting_time:
                        B2ERROR(f"Exit code file for {self.job} missing and we can't wait longer. Setting exit code to 1.")
                        exit_code = 1
                    else:
                        B2WARNING(f"Exit code file for {self.job} missing, will wait longer.")
                        return
                if exit_code:
                    jobs_info = [{"JobStatus": 6, "HoldReason": None}]  # Set to failed
                else:
                    jobs_info = [{"JobStatus": 4, "HoldReason": None}]  # Set to completed

            # If this job wasn't in the passed in condor_q output, let's try our own with the specific job_id
            if not jobs_info:
                jobs_info = HTCondor.condor_q(job_id=self.job_id, class_ads=["JobStatus", "HoldReason"])["JOBS"]

            # If no job information is returned then the job already left the queue
            # check in the history to see if it suceeded or failed
            if not jobs_info:
                try:
                    jobs_info = HTCondor.condor_history(job_id=self.job_id, class_ads=["JobStatus", "HoldReason"])["JOBS"]
                except KeyError:
                    hold_reason = "No Reason Known"

            # Still no record of it after waiting for the exit code file?
            if not jobs_info:
                jobs_info = [{"JobStatus": 6, "HoldReason": None}]  # Set to failed

            job_info = jobs_info[0]
            backend_status = job_info["JobStatus"]
            # if job is held (backend_status = 5) then report why then kill the job
            if backend_status == 5:
                hold_reason = job_info["HoldReason"]
                B2WARNING(f"{self.job} on hold because of {hold_reason}. Killing it")
                subprocess.check_output(["condor_rm", self.job_id], stderr=subprocess.STDOUT, universal_newlines=True)
                backend_status = 6
            try:
                new_job_status = self.backend_code_to_status[backend_status]
            except KeyError as err:
                raise BackendError(f"Unidentified backend status found for {self.job}: {backend_status}")
            if new_job_status != self.job.status:
                self.job.status = new_job_status

    @classmethod
    def _create_job_result(cls, job, job_id):
        """
        """
        B2INFO(f"Job ID of {job} recorded as: {job_id}")
        job.result = cls.HTCondorResult(job, job_id)

    @classmethod
    def _create_parent_job_result(cls, parent):
        parent.result = cls.HTCondorResult(parent, None)

    def can_submit(self, njobs=1):
        """
        Checks the global number of jobs in HTCondor right now (submitted or running) for this user.
        Returns True if the number is lower that the limit, False if it is higher.

        Parameters:
            njobs (int): The number of jobs that we want to submit before checking again. Lets us check if we
                are sufficiently below the limit in order to (somewhat) safely submit. It is slightly dangerous to
                assume that it is safe to submit too many jobs since there might be other processes also submitting jobs.
                So njobs really shouldn't be abused when you might be getting close to the limit i.e. keep it <=250
                and check again before submitting more.
        """
        B2DEBUG(29, "Calling HTCondor().can_submit()")
        jobs_info = self.condor_q()
        total_jobs = jobs_info["NJOBS"]
        B2INFO(f"Total jobs active in the HTCondor system is currently {total_jobs}")
        if (total_jobs + njobs) > self.global_job_limit:
            B2INFO(f"Since the global limit is {self.global_job_limit} we cannot submit {njobs} jobs until some complete.")
            return False
        else:
            B2INFO("There is enough space to submit more jobs.")
            return True

    @classmethod
    def condor_q(cls, class_ads=None, job_id="", username=""):
        """
        Simplistic interface to the `condor_q` command. lets you request information about all jobs matching the filters
        'job_id' and 'username'. Note that setting job_id negates username so it is ignored.
        The result is the JSON dictionary returned by output of the ``-json`` condor_q option.

        Parameters:
            class_ads (list[str]): A list of condor_q ClassAds that you would like information about.
                By default we give {cls.default_class_ads}, increasing the amount of class_ads increase the time taken
                by the condor_q call.
            job_id (str): String representation of the Job ID given by condor_submit during submission.
                If this argument is given then the output of this function will be only information about this job.
                If this argument is not given, then all jobs matching the other filters will be returned.
            username (str): By default we return information about only the current user's jobs. By giving
                a username you can access the job information of a specific user's jobs. By giving ``username='all'`` you will
                receive job information from all known user jobs matching the other filters. This may be a LOT of jobs
                so it isn't recommended.

        Returns:
            dict: JSON dictionary of the form:

            .. code-block:: python

              {
                "NJOBS":<number of records returned by command>,
                "JOBS":[
                        {
                         <ClassAd: value>, ...
                        }, ...
                       ]
              }
        """
        B2DEBUG(29, f"Calling HTCondor.condor_q(class_ads={class_ads}, job_id={job_id}, username={username})")
        if not class_ads:
            class_ads = cls.default_class_ads
        # Output fields should be comma separated.
        field_list_cmd = ",".join(class_ads)
        cmd_list = ["condor_q", "-json", "-attributes", field_list_cmd]
        # If job_id is set then we ignore all other filters
        if job_id:
            cmd_list.append(job_id)
        else:
            if not username:
                username = os.environ["USER"]
            # If the username is set to all it is a special case
            if username == "all":
                cmd_list.append("-allusers")
            else:
                cmd_list.append(username)
        # We get a JSON serialisable summary from condor_q. But we will alter it slightly to be more similar to other backends
        cmd = " ".join(cmd_list)
        B2DEBUG(29, f"Calling subprocess with command = '{cmd}'")
        records = subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True, shell=True)
        if records:
            records = decode_json_string(records)
        else:
            records = []
        jobs_info = {"JOBS": records}
        jobs_info["NJOBS"] = len(jobs_info["JOBS"])  # Just to avoid having to len() it in the future
        return jobs_info

    @classmethod
    def condor_history(cls, class_ads=None, job_id="", username=""):
        """
        Simplistic interface to the ``condor_history`` command. lets you request information about all jobs matching the filters
        ``job_id`` and ``username``. Note that setting job_id negates username so it is ignored.
        The result is a JSON dictionary filled by output of the ``-json`` ``condor_history`` option.

        Parameters:
            class_ads (list[str]): A list of condor_history ClassAds that you would like information about.
                By default we give {cls.default_class_ads}, increasing the amount of class_ads increase the time taken
                by the condor_q call.
            job_id (str): String representation of the Job ID given by condor_submit during submission.
                If this argument is given then the output of this function will be only information about this job.
                If this argument is not given, then all jobs matching the other filters will be returned.
            username (str): By default we return information about only the current user's jobs. By giving
                a username you can access the job information of a specific user's jobs. By giving ``username='all'`` you will
                receive job information from all known user jobs matching the other filters. This is limited to 10000 records
                and isn't recommended.

        Returns:
            dict: JSON dictionary of the form:

            .. code-block:: python

              {
                "NJOBS":<number of records returned by command>,
                "JOBS":[
                        {
                         <ClassAd: value>, ...
                        }, ...
                       ]
              }
        """
        B2DEBUG(29, f"Calling HTCondor.condor_history(class_ads={class_ads}, job_id={job_id}, username={username})")
        if not class_ads:
            class_ads = cls.default_class_ads
        # Output fields should be comma separated.
        field_list_cmd = ",".join(class_ads)
        cmd_list = ["condor_history", "-json", "-attributes", field_list_cmd]
        # If job_id is set then we ignore all other filters
        if job_id:
            cmd_list.append(job_id)
        else:
            if not username:
                username = os.environ["USER"]
            # If the username is set to all it is a special case
            if username != "all":
                cmd_list.append(username)
        # We get a JSON serialisable summary from condor_q. But we will alter it slightly to be more similar to other backends
        cmd = " ".join(cmd_list)
        B2DEBUG(29, f"Calling subprocess with command = '{cmd}'")
        records = subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True, shell=True)
        if records:
            records = decode_json_string(records)
        else:
            records = []
        jobs_info = {"JOBS": records}
        jobs_info["NJOBS"] = len(jobs_info["JOBS"])  # Just to avoid having to len() it in the future
        return jobs_info


class DIRAC(Backend):
    """
    Backend for submitting calibration processes to the grid.
    """


class BackendError(Exception):
    """
    Base exception class for Backend classes.
    """


class JobError(Exception):
    """
    Base exception class for Job objects.
    """


class SplitterError(Exception):
    """
    Base exception class for SubjobSplitter objects.
    """
