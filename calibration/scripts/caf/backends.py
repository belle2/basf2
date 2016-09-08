import os
import subprocess
import multiprocessing as mp
from collections import defaultdict
import shutil
from itertools import repeat
import glob
from .utils import method_dispatch
import pickle

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
    pass


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
