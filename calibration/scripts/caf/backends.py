import os
import subprocess
import multiprocessing as mp
from collections import defaultdict
import shutil
from itertools import repeat
import glob
from .utils import method_dispatch


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
    """

    def __init__(self):
        """
        Init method for Local Backend.
        """
        #: The size of the multiprocessing process pool.
        self.max_processes = 1

    @method_dispatch
    def submit(self, job):
        """
        Submit method of Local() backend. Submits collection jobs in subprocesses using the
        pickled basf2 paths. Captures the stdout and stderr of the jobs and places output
        files in the right calibration directories.
        """
        # Make sure the output directory of the job is created
        if not os.path.exists(job.output_dir):
            os.mkdir(job.output_dir)

        # Make sure the working directory of the job is created
        if job.working_dir and not os.path.exists(job.working_dir):
            os.mkdir(job.working_dir)

        # Get all of the requested files for the input sandbox and copy them to the working directory
        for file_pattern in job.input_sandbox_files:
            input_files = glob.glob(file_pattern)
            for file in input_files:
                shutil.copy(file, job.working_dir)

        self.run_job(job)

    @submit.register(list)
    def _(self, jobs):
        """
        Submit method of Local() that takes a list of jobs instead of just one and creates a process pool
        to run them.
        """
        # Create a Process Pool and submit the jobs to it
        with mp.Pool(self.max_processes) as pool:
            print('Starting up Pool with {0} processes'.format(self.max_processes))
            pool.map(self.submit, jobs)

    def run_job(self, job):
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

        # Once the subprocess is done, move the requested output to the output directory
        for pattern in job.output_files:
            output_files = glob.glob(os.path.join(job.working_dir, pattern))
            for file in output_files:
                shutil.move(file, job.output_dir)


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
