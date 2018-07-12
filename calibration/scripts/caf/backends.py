#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from abc import ABC, abstractmethod
import subprocess
import multiprocessing as mp
from collections import defaultdict
import shutil
from itertools import repeat
import glob
from .utils import method_dispatch
import pickle
import configparser
from .utils import decode_json_string
import pathlib

import ROOT

__all__ = ['Job', 'Backend', 'Local', 'Batch', 'LSF', 'PBS']

#: default configuration file location
default_config_file = ROOT.Belle2.FileSystem.findFile('calibration/data/backends.cfg')
#: Multiprocessing Pool used by Local Backend
_pool = None
#: Default name of the input data file passed to Backend jobs
_input_data_file_path = "input_data_files.data"


def get_input_data():
    """
    Simple pickle load of the default input data pickle file name for the CAF
    """
    with open(_input_data_file_path, 'br') as input_data_file:
        input_data = list(pickle.load(input_data_file))
    return input_data


class Job:
    """
    This generic Job object is used to tell a Backend what to do.
    This object basically holds necessary information about a process you want to submit to a `Backend`.

    Parameters:
        name (str): Simply a name to describe the Job, not used for any critical purpose in the CAF
    Keyword Arguments:
        config_file (str): Optional path to a config file containing setup for the job. Generally used for basf2 setup.
            Requires a 'BASF2' section with 'Release', 'Tools', 'SetupCmds' options.
            By default the backends.default_config_file is used.

    .. warning:: It is recommended to always use absolute paths for files when submitting a `Job`.
    """

    #: Allowed Job status dictionary. The  key is the status name and the value is its level. The lowest level
    #: out of all subjobs is the one that is the overal status of the overall job.
    statuses = {"init": 0, "submitted": 1, "running": 2, "failed": 3, "completed": 4}

    #: Job statuses that correspond to the Job being finished (successfully or not)
    exit_statuses = ["failed", "completed"]

    class SubJob():
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
            self.input_files = input_files
            #: The result object of this SubJob. Only filled once the parent `Job` object is submitted to a backend
            #: since the backend creates a special result class depending on its type.
            self.result = None
            #: Status of the subjob
            self._status = "init"

        @property
        def output_dir(self):
            """
            Getter for output_dir of SubJob. Accesses the parent Job output_dir to infer this."""
            return os.path.join(self.parent.output_dir, str(self.id))

        @property
        def working_dir(self):
            """Getter for working_dir of SubJob. Accesses the parent Job working_dir to infer this."""
            return os.path.join(self.parent.working_dir, str(self.id))

        @property
        def name(self):
            """Getter for name of SubJob. Accesses the parent Job name to infer this."""
            return "_".join((self.parent.name, str(self.id)))

        @property
        def subjobs(self):
            """Getter for subjobs of SubJob object. Always returns None, used to prevent accessing the parent job by mistake"""
            return None

        def ready(self):
            """
            Returns the ready() status from the result of this SubJob
            """
            return self.result.ready()

        @property
        def status(self):
            """
            Returns the status of this SubJob.
            """
            return self._status

        @status.setter
        def status(self, status):
            """
            """
            B2INFO("Setting {} status to {}".format(self.name, status))
            self._status = status

        @property
        def max_files_per_subjob(self):
            """Always returns -1, used to prevent accessing the parent job by mistake"""
            return -1

        def __getattr__(self, attribute):
            """
            Since a SubJob uses attributes from the parent Job, everything simply accesses the Job attributes
            unless otherwise specified.
            """
            return getattr(self.parent, attribute)

        def __repr__(self):
            """
            """
            return self.name

    def __init__(self, name, config_file=""):
        """
        """
        #: Job object's name. Only descriptive, not necessarily unique.
        self.name = name
        #: Files to be tarballed and sent along with the job (NOT the input root files)
        self.input_sandbox_files = []
        #: Working directory of the job (str). Default is '.', mostly used in Local() backend
        self.working_dir = '.'
        #: Output directory (str), where we will download our output_files to. Default is '.'
        self.output_dir = '.'
        #: Files that we produce during the job and want to be returned. Can use wildcard (*)
        self.output_patterns = []
        #: Command and arguments as a list that wil be run by the job on the backend
        self.cmd = []
        #: Input files to job, a list of these is copied to the working directory.
        self.input_files = []
        #: Bash commands to run before the main self.cmd (mainly used for batch system setup)
        self.setup_cmds = []
        #: ConfigParser object that sets the defaults (mainly for basf2 setup) and is used as the store
        #: of configuration variables. Some other attributes reference this object via properties.
        self.config = configparser.ConfigParser()
        if not config_file:
            config_file = default_config_file
        self.config.read(config_file)
        #: Config dictionary for the backend to use when submitting the job. Saves us from having multiple attributes that may or
        #: may not be used.
        self.backend_args = {}
        #: Maximum number of files to place in a subjob (Not applicable to Local processing). -1 means don't split into subjobs
        self.max_files_per_subjob = -1
        #: dict of subjobs assigned to this job
        self.subjobs = {}
        #: The result object of this Job. Only filled once the job is submitted to a backend since the backend creates a special
        #: result class depending on its type. This is also only filled for an overall `Job()` if there are no subjobs.
        #: If there are subjobs, those will contain the result instead.
        self.result = None
        #: The actual status of the overall `Job`. The property handles querying for the subjob status to set this
        self._status = "init"

    def __repr__(self):
        """
        Representation of Job class (what happens when you print a Job() instance)
        """
        return self.name

    def ready(self):
        """
        Returns whether or not the Job has finished. If the job has subjobs then it will return true when they are all finished.
        It will return False as soon as it hits the first failure. Meaning that you cannot guarantee that all subjobs will have
        their status updated when calling this method. Instead use :py:meth:`update_status` to update all statuses if necessary.
        """
        if self.subjobs:
            for subjob in self.subjobs.values():
                if not subjob.ready():
                    return False
            else:
                return True
        else:
            return self.result.ready()

    def update_status(self):
        """
        Calls :py:meth:`update_status` on the job, or all its subjobs to force update the status of all of them.
        """
        if self.subjobs:
            for subjob in self.subjobs.values():
                subjob.result.update_status()
        else:
            self.result.update_status()
        return self.status

    def add_basf2_setup(self):
        """
        Creates a list of lines to write into a bash shell script that will setup basf2.
        Extends the self.setup_cmds attribute when called instead of replacing it.
        """
        setup_cmds = self.config['BASF2']['SetupCmds']
        self.setup_cmds.extend(decode_json_string(self.config['BASF2']['SetupCmds']))

    def create_subjob(self, i):
        """
        Creates a subjob Job object that references that parent Job.
        Returns the SubJob object at the end.
        """
        if i not in self.subjobs:
            B2INFO('Creating Job({}).Subjob({})'.format(self.name, i))
            subjob = Job.SubJob(self, i)
            self.subjobs[i] = subjob
            return subjob
        else:
            B2WARNING("Job({}) already contains SubJob({})! This will not be created.".format(self.name, i))

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
        """
        B2INFO("Setting {} status to {}".format(self.name, status))
        self._status = status

    @property
    def basf2_release(self):
        """
        Getter for basf2 release version string. Pulls in the value from the configparser
        """
        return self.config.get("BASF2", "Release")

    @basf2_release.setter
    def basf2_release(self, release):
        """
        Setter for basf2 release version string, it actually sets the value in the configparser
        """
        self.config.set("BASF2", "Release", release)

    @property
    def basf2_tools(self):
        """
        Getter for basf2 tools location path. Pulls in the value from the configparser
        """
        return self.config.get("BASF2", "Tools")

    @basf2_tools.setter
    def basf2_tools(self, tools_location):
        """
        Setter for basf2 tools location path, it actually sets the value in the configparser
        """
        self.config.set("BASF2", "Tools", tools_location)

    def post_process(self):
        """
        Does clean up tasks after the main job has finished. Mainly downloading output to the
        correct output location.
        """
        # Once the subprocess is done, move the requested output to the output directory
        if not self.subjobs:
            for pattern in self.output_patterns:
                output_files = glob.glob(os.path.join(self.working_dir, pattern))
                for file_name in output_files:
                    shutil.move(file_name, self.output_dir)
        else:
            for subjob in job.subjobs:
                subjob.post_process()


class Backend(ABC):
    """
    Abstract base class for a valid backend.
    Classes derived from this will implement their own submission of basf2 jobs
    to whatever backend they describe.
    Some common methods/attributes go into this base class.
    """
    #: Default submission script name
    submit_script = "submit.sh"

    def __init__(self):
        """
        """
        pass

    @abstractmethod
    def submit(self, job):
        """
        Base method for submitting collection jobs to the backend type. This MUST be
        implemented for a correctly written backend class deriving from Backend().
        """
        pass

    @staticmethod
    def _dump_input_data(job):
        if job.input_files:
            with open(str(pathlib.Path(job.working_dir, 'input_data_files.data')), 'bw') as input_data_file:
                pickle.dump(job.input_files, input_data_file)

    @staticmethod
    def _add_setup(job, batch_file):
        """
        Adds setup lines to the shell script file.
        """
        for line in job.setup_cmds:
            print(line, file=batch_file)


class Result():
    """
    Base class for Result objecs. A Result is created for each `Job` (or `Job.SubJob`) object
    submitted to a backend. It provides a way to query a job's status to find out if it's ready.
    """

    def __init__(self, job):
        """
        Pass in the job object to allow the result to access the job's properties and do post-processing.
        """
        #: Job object for result
        self.job = job
        #: Quicker way to know if it's ready once it has been checked
        self._is_ready = False

    def ready(self):
        """
        Returns whether or not this job result is ready. Uses the derived class's `update_status`
        method to check the status. Only runs `update_status` if it has never returned True before,
        otherwise it just returns True.
        """
        B2DEBUG(29, "Calling {}.result.ready()".format(self.job.name))
        if self._is_ready:
            return True
        self.update_status()
        if self.job.status in self.job.exit_statuses:
            self._is_ready = True
            return True
        else:
            return False

    def update_status(self):
        """
        Update the job's status by calling bjobs.
        """
        raise NotImplementedError

    def post_process(self):
        """
        Does clean up tasks after the main job has finished. Mainly downloading output to the
        correct output location.
        """
        # Once the subprocess is done, move the requested output to the output directory
        for pattern in self.job.output_patterns:
            output_files = glob.glob(os.path.join(self.job.working_dir, pattern))
            for file_name in output_files:
                shutil.move(file_name, self.job.output_dir)


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

    def __init__(self, max_processes=1):
        """
        """
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

        def update_status(self):
            """
            Update the job's status by calling the result object.
            """
            if self.result.ready() and (self.job.status not in self.job.exit_statuses):
                return_code = self.result.get()
                if return_code:
                    self.job.status = "failed"
                else:
                    self.job.status = "completed"

    def join(self):
        """
        Closes and joins the Pool, letting you wait for all results currently
        still processing.
        """
        global _pool
        B2INFO('Joining Process Pool, waiting for results to finish')
        _pool.close()
        _pool.join()
        B2INFO('Process Pool joined.')

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
        global _pool
        if _pool:
            self.join()
        B2INFO('Starting up new Pool with {0} processes'.format(self.max_processes))
        _pool = mp.Pool(processes=self.max_processes)

    @method_dispatch
    def submit(self, job):
        """
        Submit method of Local() backend. Mostly setup here creating the right directories.
        """
        # Make sure the output directory of the job is created
        job_output_path = pathlib.Path(job.output_dir)
        job_output_path.mkdir(parents=True, exist_ok=True)
        # Make sure the working directory of the job is created
        job_wd_path = pathlib.Path(job.working_dir)
        job_wd_path.mkdir(parents=True, exist_ok=True)

        global _pool
        if isinstance(job, Job):
            if job.max_files_per_subjob == 0:
                B2FATAL("When submitting Job({}) files per subjob was 0!".format(job.name))

            # Check if we have any valid input files
            existing_input_files = set()
            for input_file_pattern in job.input_files:
                if input_file_pattern[:7] != "root://":
                    input_files = glob.glob(input_file_pattern)
                    if not input_files:
                        B2WARNING("No files matching {0} can be found, it will be skipped!".format(input_file_pattern))
                    else:
                        for file_path in input_files:
                            file_path = os.path.abspath(file_path)
                            if os.path.isfile(file_path):
                                existing_input_files.add(file_path)
                else:
                    B2INFO(("Found an xrootd file path {0} it will not be checked for validity"
                            " before collector submission.".format(input_file_pattern)))
                    existing_input_files.add(input_file_pattern)

            if not existing_input_files:
                B2INFO("No valid input file paths found for job {0}".format(job.name))

            if job.max_files_per_subjob > 0 and existing_input_files:
                import itertools

                def grouper(n, iterable):
                    it = iter(iterable)
                    while True:
                        chunk = tuple(itertools.islice(it, n))
                        if not chunk:
                            return
                        yield chunk

                for i, subjob_input_files in enumerate(grouper(job.max_files_per_subjob, existing_input_files)):
                    subjob = job.create_subjob(i)
                    subjob.input_files = subjob_input_files

            if not job.subjobs:
                # Get all of the requested files for the input sandbox and copy them to the working directory
                for file_pattern in job.input_sandbox_files:
                    input_files = glob.glob(file_pattern)
                    for file_path in input_files:
                        if os.path.isdir(file_path):
                            shutil.copytree(file_path, os.path.join(job.working_dir, os.path.split(file_path)[1]))
                        else:
                            shutil.copy(file_path, job.working_dir)

                self._dump_input_data(job)
                with open(os.path.join(job.working_dir, self.submit_script), "w") as batch_file:
                    self._add_setup(job, batch_file)
                    print(" ".join(job.cmd), file=batch_file)
                B2INFO("Submitting Job({})".format(job.name))
                job.result = Local.LocalResult(job,
                                               _pool.apply_async(self.run_job,
                                                                 (job.name,
                                                                  job.working_dir,
                                                                  job.output_dir,
                                                                  self.submit_script)
                                                                 )
                                               )
                B2INFO('Job {0} submitted'.format(job.name))
            else:
                self.submit(list(job.subjobs.values()))

        elif isinstance(job, Job.SubJob):
            # Get all of the requested files for the input sandbox and copy them to the working directory
            for file_pattern in job.input_sandbox_files:
                input_files = glob.glob(file_pattern)
                for file_path in input_files:
                    if os.path.isdir(file_path):
                        shutil.copytree(file_path, os.path.join(job.working_dir, os.path.split(file_path)[1]))
                    else:
                        shutil.copy(file_path, job.working_dir)

            self._dump_input_data(job)
            with open(os.path.join(job.working_dir, self.submit_script), "w") as batch_file:
                self._add_setup(job, batch_file)
                print(" ".join(job.cmd), file=batch_file)
            B2INFO("Submitting Job({})".format(job.name))
            job.result = Local.LocalResult(job,
                                           _pool.apply_async(self.run_job,
                                                             (job.name,
                                                              job.working_dir,
                                                              job.output_dir,
                                                              self.submit_script)
                                                             )
                                           )
            B2INFO('Job {0} submitted'.format(job.name))

    @submit.register(list)
    def _(self, jobs):
        """
        Submit method of Local() that takes a list of jobs instead of just one and submits each one.
        """
        # Submit the jobs to PBS
        for job in jobs:
            self.submit(job)
        B2INFO('All Requested Jobs Submitted')

    @staticmethod
    def run_job(name, working_dir, output_dir, script):
        """
        The function that is used by multiprocessing.Pool.map during process creation. This runs a
        shell command in a subprocess and captures the stdout and stderr of the subprocess to files.
        """
        B2INFO('Starting Sub Process: {0}'.format(name))
        from subprocess import PIPE, STDOUT, Popen
        stdout_file_path = os.path.join(working_dir, 'stdout')
        # Create unix command to redirect stdour and stderr
        B2INFO('Log files for SubProcess {0} visible at:\n\t{1}'.format(name, stdout_file_path))
        with open(stdout_file_path, 'w', buffering=1) as f_out:
            with Popen(["/bin/bash", "-l", script],
                       stdout=PIPE,
                       stderr=STDOUT,
                       bufsize=1,
                       universal_newlines=True,
                       cwd=working_dir) as p:
                for line in p.stdout:
                    print(line, end='', file=f_out)
        B2INFO('Sub Process {0} Finished.'.format(name))
        return p.returncode


class Batch(Backend):
    """
    Abstract Base backend for submitting to a local batch system. Batch system specific commands should be implemented
    in a derived class. Do not use this class directly!
    """
    #: default section name in config file, should be implemented in the derived class
    default_config_section = ""
    #: Shell command to submit a script, should be implemented in the derived class
    submission_cmds = []

    def __init__(self):
        """
        Init method for Batch Backend. Does default setup based on config file.
        """
        #: ConfigParser object containing some setup config for this class
        self.config = configparser.ConfigParser()
        self.config.read(default_config_file)

    def _add_batch_directives(self, job, file):
        """
        Should be implemented in a derived class to write a batch submission script to the job.working_dir.
        You should think about where the stdout/err should go, and set the queue name.
        """
        raise NotImplementedError(("Need to implement a _add_batch_directives(self, job, file) "
                                   "method in {} backend.".format(self.__class__.__name__)))

    @classmethod
    @abstractmethod
    def _submit_to_batch(cls, cmd):
        """
        Do the actual batch submission command and collect the output to find out the job id for later monitoring.
        """
        pass

    @method_dispatch
    def submit(self, job):
        """
        Submit method of Batch backend. Should take job object, create needed directories, create batch script,
        and send it off with the batch submission command, applying the correct options (default and user requested.)

        Should set a Result object as an attribute of the job. Result object should allow a 'ready' member method to
        be called from it which queries the batch system and the job id about whether or not the job has finished.
        """
        B2INFO("Submitting job {}".format(job.name))
        # Make sure the output directory of the job is created
        job_output_path = pathlib.Path(job.output_dir)
        job_output_path.mkdir(parents=True, exist_ok=True)
        # Make sure the working directory of the job is created
        job_wd_path = pathlib.Path(job.working_dir)
        job_wd_path.mkdir(parents=True, exist_ok=True)

        if isinstance(job, Job):
            if 'queue' not in job.backend_args:
                job.backend_args['queue'] = self.config[self.default_config_section]["Queue"]
                B2INFO('Setting Job queue {}'.format(job.backend_args['queue']))

            if job.max_files_per_subjob == 0:
                B2FATAL("When submitting Job({}) files per subjob was 0!".format(job.name))

            # Check if we have any valid input files
            existing_input_files = set()
            for input_file_pattern in job.input_files:
                if input_file_pattern[:7] != "root://":
                    input_files = glob.glob(input_file_pattern)
                    if not input_files:
                        B2WARNING("No files matching {0} can be found, it will be skipped!".format(input_file_pattern))
                    else:
                        for file_path in input_files:
                            file_path = os.path.abspath(file_path)
                            if os.path.isfile(file_path):
                                existing_input_files.add(file_path)
                else:
                    B2INFO(("Found an xrootd file path {0} it will not be checked for validity"
                            " before collector submission.".format(input_file_pattern)))
                    existing_input_files.add(input_file_pattern)

            if not existing_input_files:
                B2INFO("No valid input file paths found for job {0}".format(job.name))

            if job.max_files_per_subjob > 0 and existing_input_files:
                import itertools

                def grouper(n, iterable):
                    it = iter(iterable)
                    while True:
                        chunk = tuple(itertools.islice(it, n))
                        if not chunk:
                            return
                        yield chunk

                for i, subjob_input_files in enumerate(grouper(job.max_files_per_subjob, existing_input_files)):
                    subjob = job.create_subjob(i)
                    subjob.input_files = subjob_input_files

            if not job.subjobs:
                # Get all of the requested files for the input sandbox and copy them to the working directory
                for file_pattern in job.input_sandbox_files:
                    input_files = glob.glob(file_pattern)
                    for file_path in input_files:
                        if os.path.isdir(file_path):
                            shutil.copytree(file_path, os.path.join(job.working_dir, os.path.split(file_path)[1]))
                        else:
                            shutil.copy(file_path, job.working_dir)

                self._dump_input_data(job)
                with open(os.path.join(job.working_dir, self.submit_script), "w") as batch_file:
                    self._add_batch_directives(job, batch_file)
                    self._add_setup(job, batch_file)
                    print(" ".join(job.cmd), file=batch_file)
                B2INFO("Submitting Job({})".format(job.name))

                script_path = os.path.join(job.working_dir, self.submit_script)
                cmd = self._create_cmd(script_path)
                output = self._submit_to_batch(cmd)
                self._create_job_result(job, output)
                job.status = "submitted"
            else:
                self.submit(list(job.subjobs.values()))

        elif isinstance(job, Job.SubJob):
            # Get all of the requested files for the input sandbox and copy them to the working directory
            for file_pattern in job.input_sandbox_files:
                input_files = glob.glob(file_pattern)
                for file_path in input_files:
                    if os.path.isdir(file_path):
                        shutil.copytree(file_path, os.path.join(job.working_dir, os.path.split(file_path)[1]))
                    else:
                        shutil.copy(file_path, job.working_dir)

            self._dump_input_data(job)
            with open(os.path.join(job.working_dir, self.submit_script), "w") as batch_file:
                self._add_batch_directives(job, batch_file)
                self._add_setup(job, batch_file)
                print(" ".join(job.cmd), file=batch_file)
            B2INFO("Submitting SubJob({})".format(job.name))
            script_path = os.path.join(job.working_dir, self.submit_script)
            cmd = self._create_cmd(script_path)
            output = self._submit_to_batch(cmd)
            self._create_job_result(job, output)
            job.status = "submitted"

    @submit.register(list)
    def _(self, jobs):
        """
        Submit method of Batch Backend that takes a list of jobs instead of just one and submits each one.
        """
        B2INFO("Submitting a list of jobs to a Batch backend")
        for job in jobs:
            self.submit(job)
        B2INFO('All Requested Jobs Submitted')

    @classmethod
    @abstractmethod
    def _create_job_result(cls, job, batch_output):
        """
        """
        pass

    @classmethod
    @abstractmethod
    def _create_cmd(cls, job):
        """
        """
        pass


class PBS(Batch):
    """
    Backend for submitting calibration processes to a qsub batch system
    """
    #: Working directory directive
    cmd_wkdir = "#PBS -d"
    #: stdout file directive
    cmd_stdout = "#PBS -o"
    #: stderr file directive
    cmd_stderr = "#PBS -e"
    #: Walltime directive
    cmd_walltime = "#PBS -l walltime="
    #: Queue directive
    cmd_queue = "#PBS -q"
    #: Job name directive
    cmd_name = "#PBS -N"
    #: default PBS section name in config file
    default_config_section = "PBS"
    #: Shell command to submit a script
    submission_cmds = ["qsub"]

    def _add_batch_directives(self, job, batch_file):
        """
        Creates the bash shell script that qsub will call. Includes PBD directives
        and basf2 setup.
        """
        if "queue" in job.backend_args:
            batch_queue = job.backend_args["queue"]
        else:
            batch_queue = self.config["Queue"]
        print("#!/bin/bash", file=batch_file)
        print("# --- Start PBS ---", file=batch_file)
        print(" ".join([PBS.cmd_queue, batch_queue]), file=batch_file)
        print(" ".join([PBS.cmd_name, job.name]), file=batch_file)
        print(" ".join([PBS.cmd_wkdir, job.working_dir]), file=batch_file)
        print(" ".join([PBS.cmd_stdout, os.path.join(job.working_dir, "stdout")]), file=batch_file)
        print(" ".join([PBS.cmd_stderr, os.path.join(job.working_dir, "stderr")]), file=batch_file)
        print("# --- End PBS ---", file=batch_file)

    @classmethod
    def _create_job_result(cls, job, batch_output):
        """
        """
        job_id = batch_output.replace("\n", "")
        B2INFO("Job ID of Job({}) recorded as: {}".format(job.name, job_id))
        job.result = cls.PBSResult(job, job_id)

    @classmethod
    def _create_cmd(cls, script_path):
        """
        """
        submission_cmd = cls.submission_cmds[:]
        submission_cmd.append(script_path)
        return submission_cmd

    @classmethod
    def _submit_to_batch(cls, cmd):
        """
        Do the actual batch submission command and collect the output to find out the job id for later monitoring.
        """
        sub_out = subprocess.check_output(cmd, stderr=subprocess.STDOUT, universal_newlines=True, env={})
        return sub_out

    class PBSResult(Result):
        """
        Simple class to help monitor status of jobs submitted by `PBS` Backend.

        You pass in a `Job` object (or `SubJob`) and job id from a bsub command.
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
            #: job id given by LSF
            self.job_id = job_id

        def update_status(self):
            """
            Update the job's status by calling bjobs.
            """
            B2DEBUG(29, "Calling {}.result.update_status()".format(self.job.name))
            # Run qstat
            try:
                output = subprocess.check_output(["qstat", "-r", self.job_id], stderr=subprocess.STDOUT, universal_newlines=True)
            except subprocess.CalledProcessError as cpe:
                # If the job isn't available anymore, let's assume it finished
                first_line = cpe.output.splitlines()[0]
                if "Unknown Job Id Error" in first_line:
                    new_job_status = self.backend_code_to_status["FINISHED"]
                    if self.job.status != new_job_status:
                        self.job.status = new_job_status
                    return
                else:
                    B2ERROR("'qstat' command failed for {} but for an unknown reason.".format(self.job.name))
                    # If it was some other error just re-raise
                    raise cpe
            # If it didn't error, parse the output
            backend_status = self._get_status_from_output(output)
            try:
                new_job_status = self.backend_code_to_status[backend_status]
            except KeyError as err:
                raise BackendError("Unidentified backend status found for Job({}): {}".format(self.job.name, backend_status))
            if new_job_status != self.job.status:
                self.job.status = new_job_status

        def _get_status_from_output(self, output):
            info_line = output.splitlines()[5]
            status_code = info_line.split()[-2]
            return status_code


class LSF(Batch):
    """
    Backend for submitting calibration processes to a qsub batch system
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
    #: default LSF section name in config file
    default_config_section = "LSF"
    #: Shell command to submit a script
    submission_cmds = ["bsub", "<"]

    def _add_batch_directives(self, job, batch_file):
        """
        Adds LSF BSUB directives for the job to a script
        """
        if "queue" in job.backend_args:
            batch_queue = job.backend_args["queue"]
        else:
            batch_queue = self.config["Queue"]
        print("#!/bin/bash", file=batch_file)
        print("# --- Start LSF ---", file=batch_file)
        print(" ".join([LSF.cmd_queue, batch_queue]), file=batch_file)
        print(" ".join([LSF.cmd_name, job.name]), file=batch_file)
        print(" ".join([LSF.cmd_wkdir, job.working_dir]), file=batch_file)
        print(" ".join([LSF.cmd_stdout, os.path.join(job.output_dir, "stdout")]), file=batch_file)
        print(" ".join([LSF.cmd_stderr, os.path.join(job.output_dir, "stderr")]), file=batch_file)
        print("# --- End LSF ---", file=batch_file)

    @classmethod
    def _create_cmd(cls, script_path):
        """
        """
        submission_cmd = cls.submission_cmds[:]
        submission_cmd.append(script_path)
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
            Update the job's status by calling bjobs.
            """
            B2DEBUG(29, "Calling {}.result.update_status()".format(self.job.name))
            output = subprocess.check_output(["bjobs", self.job_id], stderr=subprocess.STDOUT, universal_newlines=True)
            backend_status = self._get_status_from_output(output)
            try:
                new_job_status = self.backend_code_to_status[backend_status]
            except KeyError as err:
                raise BackendError("Unidentified backend status found for Job({}): {}".format(self.job, backend_status))
            if new_job_status != self.job.status:
                self.job.status = new_job_status

        def _get_status_from_output(self, output):
            if output == "Job <{}> is not found\n".format(self.job_id):
                return "FINISHED"
            else:
                stat_line = output.split("\n")[1]
                status = stat_line.split()[2]
                return status

    @classmethod
    def _create_job_result(cls, job, batch_output):
        """
        """
        job_id = batch_output.split(" ")[1]
        for wrap in ["<", ">"]:
            job_id = job_id.replace(wrap, "")
        B2INFO("Job ID of Job({}) recorded as: {}".format(job.name, job_id))
        job.result = cls.LSFResult(job, job_id)


class DIRAC(Backend):
    """
    Backend for submitting calibration processes to the grid
    """
    pass


class BackendError(Exception):
    """
    Base exception class for this module
    """
    pass
