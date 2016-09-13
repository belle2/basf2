from basf2 import *
import subprocess
import multiprocessing as mp
from collections import defaultdict
import shutil
from itertools import repeat
import glob
from .utils import method_dispatch
import pickle
import configparser

import ROOT
# For weird reasons, a multiprocessing pool doesn't work properly as a class attribute
# So we make it a module variable. Should be fine as starting up multiple pools
# is not recommended anyway. Will check this again someday.
pool = None


class Backend():
    """
    Base class for backend of CAF.
    Classes derived from this will implement their own submission of basf2 jobs
    to whatever backend they describe. Common methods/attributes go here.
    """

    def submit(self):
        """
        Base method for submitting collection jobs to the backend type. This MUST be
        implemented for a correctly written backend class deriving from Backend().
        """
        raise NotImplementedError('Need to implement a submit() method in {} backend.'.format(self.__class__.__name__))


class Local(Backend):
    """
    Backend for local calibration processes i.e. on the same machine but in a subprocess.
    Attributes:
        * max_processes=<int> Specifies the size of the process pool that spawns the subjobs.
        It's the maximium simultaneous subjobs. Try not to specify a large number or a number
        larger than the number of cores (-1?). Won't crash the program but it will slow down
        and negatively impact performance. Default = 1

    Note that you should call the self.join() method to close the pool and wait for any
    running processes to finish. Once you've called join you will have to set up a new
    instance of this backend (or set max_processes again) to create a new pool.
    If you don't call self.join() and don't create a join yourself somewhere then the main
    python process might end before your pool is done.
    """

    def __init__(self, max_processes=1):
        """
        Init method for Local Backend. Spawns a multiprocessing pool based on
        max_processes argument (default=1)
        """
        #: The size of the multiprocessing process pool.
        self.max_processes = max_processes

    def join(self):
        """
        Closes and joins the Pool, letting you wait for all results currently
        still processing.
        """
        print('Joining Process Pool, waiting for results to finish')
        global pool
        pool.close()
        pool.join()
        print('Process Pool joined.')

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
        global pool
        if pool:
            self.join()
        print('Starting up new Pool with {0} processes'.format(self.max_processes))
        pool = mp.Pool(self.max_processes)

    @method_dispatch
    def submit(self, job):
        """
        Submit method of Local() backend. Mostly setup here creating the right directories.
        """
        global pool
        # Submit the jobs to owned Pool
        print('Job Submitting:', job.name)
        # Make sure the output directory of the job is created
        if not os.path.exists(job.output_dir):
            os.makedirs(job.output_dir)

        # Make sure the working directory of the job is created
        if job.working_dir and not os.path.exists(job.working_dir):
            os.makedirs(job.working_dir)

        # Get all of the requested files for the input sandbox and copy them to the working directory
        for file_pattern in job.input_sandbox_files:
            input_files = glob.glob(file_pattern)
            for file_path in input_files:
                if os.path.isdir(file_path):
                    shutil.copytree(file_path, os.path.join(job.working_dir, os.path.split(file_path)[1]))
                else:
                    shutil.copy(file_path, job.working_dir)

        # Check if we have any valid input files
        existing_input_files = []
        for input_file_path in job.input_files:
            if input_file_path[:7] != "root://":
                if os.path.exists(input_file_path):
                    existing_input_files.append(input_file_path)
                else:
                    print("Requested local input file {0} can't be found, it will be skipped!".format(input_file_path))
            else:
                existing_input_files.append(input_file_path)

        if existing_input_files:
            # Now make a python file in our input sandbox containing a list of these valid files
            with open(os.path.join(job.working_dir, 'input_data_files.data'), 'bw') as input_data_file:
                pickle.dump(existing_input_files, input_data_file)

        result = pool.apply_async(Local.run_job, (job,))
        print('Job {0} submitted'.format(job.name))
        return result

    @submit.register(list)
    def _(self, jobs):
        """
        Submit method of Local() that takes a list of jobs instead of just one and creates a process pool
        to run them.
        """
        results = []
        # Submit the jobs to owned Pool
        for job in jobs:
            results.append(self.submit(job))
        print('Jobs submitted')
        return results

    @staticmethod
    def run_job(job):
        """
        The function that is used by multiprocessing.Pool.map during process creation. This runs a
        shell command in a subprocess and captures the stdout and stderr of the subprocess to files.
        """
        stdout_file_path = os.path.join(job.output_dir, 'stdout')
        stderr_file_path = os.path.join(job.output_dir, 'stderr')
        # Open the stdout and stderr for redirection of subprocess
        with open(stdout_file_path, 'w') as f_out, open(stderr_file_path, 'w') as f_err:
            print('Starting Sub Process: {0}'.format(job.name))
            subprocess.run(job.cmd, shell=False, stdout=f_out, stderr=f_err, cwd=job.working_dir)
            print('Sub Process {0} Finished.'.format(job.name))

        # Once the subprocess is done, move the requested output to the output directory
        # print('Moving any output files of process {0} to output directory {1}'.format(job.name, job.output_dir))
        for pattern in job.output_patterns:
            output_files = glob.glob(os.path.join(job.working_dir, pattern))
            for file_name in output_files:
                shutil.move(file_name, job.output_dir)
                # print('moving', file_name, 'to', job.output_dir)


class PBS(Backend):
    """
    Backend for submitting calibration processes to a qsub batch system
    """
    cmd_wkdir = "#PBS -d"
    cmd_stdout = "#PBS -o"
    cmd_stderr = "#PBS -e"
    cmd_walltime = "#PBS -l walltime="
    cmd_queue = "#PBS -q"
    cmd_name = "#PBS -N"

    default_config_file = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
    default_config_section = "PBS"

    required_config = ["Queue", "Release"]

    def __init__(self, config_file_path="", section=""):
        """
        Init method for PBS Backend. Does default setup based on config file.
        """
        if config_file_path and section:
            self.load_from_config(config_file_path, section)
        else:
            self.load_from_config(PBS.default_config_file, PBS.default_config_section)
        self.basf2_setup = None

    def load_from_config(self, config_file_path, section):
        """
        Takes a config file path (string) and the relevant section name (string).
        Checks that the required setup options are set. When loading from a config file you
        can't have a partially complete list of required options.

        If you want to modify the default setup try changing the public attributes directly.
        """
        if config_file_path and section:
            if os.path.exists(config_file_path):
                cp = configparser.ConfigParser()
                cp.read(config_file_path)
            else:
                B2FATAL("Attempted to load a config file that doesn't exist! {0}".format(config_file_path))
            if cp.has_section(section):
                for option in PBS.required_config:
                    if not cp.has_option(section, option):
                        B2FATAL(("Tried to load config file but required option {0} "
                                 "not found in section {1}".format(option, section)))
            else:
                B2FATAL("Tried to load config file but required section {0} not found.".format(section))
        self._update_from_config(cp, section)

    def _update_from_config(self, config, section):
        """
        Sets attributes from a ConfigParser object. Checks for required options should already be done
        so we can just use them here.
        """
        #: queue name for PBS submission
        self.queue = config[section]["Queue"]
        #: basf2 release path
        self.release = config[section]["Release"]

    def _generate_basf2_setup(self):
        """
        Creates a list of lines to write into a bash shell script that will setup basf2.
        Will not be called if the user has set the pbs.basf2_setup attribute themselves.
        """
        #: A list of string lines that define how basf2 will be setup on the worker node.
        #: Can be set explicitly by the user or ignored if the default/user defined config file
        #: is good enough
        self.basf2_setup = ["SETUPBELLE2_CVMFS=/cvmfs/belle.cern.ch/tools.new/setup_belle2\n",
                            "CAF_RELEASE_LOCATION="+self.release+"\n",
                            "source $SETUPBELLE2_CVMFS\n",
                            "pushd $CAF_RELEASE_LOCATION > /dev/null\n",
                            "setuprel\n",
                            "popd > /dev/null\n"]

    def _generate_pbs_script(self, job):
        """
        Creates the bash shell script that qsub will call. Includes PBD directives
        and basf2 setup.
        """
        with open(os.path.join(job.working_dir, "submit.sh"), "w") as batch_file:
            batch_file.write("#!/bin/bash\n")
            batch_file.write("# --- Start PBS ---\n")
            batch_file.write(" ".join([PBS.cmd_queue, self.queue])+"\n")
            batch_file.write(" ".join([PBS.cmd_name, job.name])+"\n")
            batch_file.write(" ".join([PBS.cmd_wkdir, job.working_dir])+"\n")
            batch_file.write(" ".join([PBS.cmd_stdout, os.path.join(job.output_dir, "stdout")])+"\n")
            batch_file.write(" ".join([PBS.cmd_stderr, os.path.join(job.output_dir, "stderr")])+"\n")
            batch_file.write("# --- End PBS ---\n")
            self._add_basf2_setup(batch_file)
            batch_file.write(" ".join(job.cmd)+"\n")

    def _add_basf2_setup(self, file):
        """
        Adds basf2 setup lines to the PBS shell script file.
        """
        for line in self.basf2_setup:
            file.write(line)

    class Result():
        """
        Simple class to help monitor status of jobs submitted by CAF backend to PBS.

        You pass in a Job ID from a qsub command and when you call the 'ready' method
        it runs qstat to see whether or not the job has finished.
        """

        def __init__(self, job_id):
            """
            """
            self.job_id = job_id

        def ready(self):
            """
            Function that queries qstat to check for jobs that are still running.
            """
            try:
                d = subprocess.check_output(["qstat", self.job_id], stderr=subprocess.STDOUT, universal_newlines=True)
            except subprocess.CalledProcessError as cpe:
                first_line = cpe.output.splitlines()[0]
                if "Unknown Job Id Error" in first_line:
                    return True
            else:
                return False

    @method_dispatch
    def submit(self, job):
        """
        Submit method of PBS backend. Should take job object, create needed directories, create PBS script,
        and send it off with qsub applying the correct options (default and user requested.)

        Should return a Result object that allows a 'ready' member method to be called from it which queries
        the PBS system and the job about whether or not the job has finished.
        """
        print('Job Submitting:', job.name)
        # Make sure the output directory of the job is created
        if not os.path.exists(job.output_dir):
            os.makedirs(job.output_dir)

        # Make sure the working directory of the job is created
        if job.working_dir and not os.path.exists(job.working_dir):
            os.makedirs(job.working_dir)

        # Get all of the requested files for the input sandbox and copy them to the working directory
        for file_pattern in job.input_sandbox_files:
            input_files = glob.glob(file_pattern)
            for file_path in input_files:
                if os.path.isdir(file_path):
                    shutil.copytree(file_path, os.path.join(job.working_dir, os.path.split(file_path)[1]))
                else:
                    shutil.copy(file_path, job.working_dir)

        # Check if we have any valid input files
        existing_input_files = []
        for input_file_path in job.input_files:
            if input_file_path[:7] != "root://":
                if os.path.exists(input_file_path):
                    existing_input_files.append(input_file_path)
                else:
                    print("Requested local input file {0} can't be found, it will be skipped!".format(input_file_path))
            else:
                existing_input_files.append(input_file_path)

        if existing_input_files:
            # Now make a python file in our input sandbox containing a list of these valid files
            with open(os.path.join(job.working_dir, 'input_data_files.data'), 'bw') as input_data_file:
                pickle.dump(existing_input_files, input_data_file)

        # If the user hasn't explicitly set the basf2_setup attribute, then do the default here from the config values
        if not self.basf2_setup:
            self._generate_basf2_setup()
        self._generate_pbs_script(job)
        result = self._submit_to_qsub(job)
        return result

    def _submit_to_qsub(self, job):
        """
        Do the actual qsub command and collect the output to find out the job id for later monitoring.
        """
        script_path = os.path.join(job.working_dir, "submit.sh")
        qsub_out = subprocess.check_output(["qsub", script_path], stderr=subprocess.STDOUT, universal_newlines=True)
        qsub_out = qsub_out.replace("\n", "")
        result = PBS.Result(qsub_out)
        return result

    @submit.register(list)
    def _(self, jobs):
        """
        Submit method of PBS() that takes a list of jobs instead of just one and submits each one
        with qsub. Possibly with multiple submissions based on input file splitting.
        """
        results = []
        # Submit the jobs to PBS
        for job in jobs:
            results.append(self.submit(job))
        print('Jobs submitted')
        return results


class LSF(Backend):
    """
    Backend for submitting calibration processes to a bsub batch system
    """
    pass


class DIRAC(Backend):
    """
    Backend for submitting calibration processes to the grid
    """
    pass
