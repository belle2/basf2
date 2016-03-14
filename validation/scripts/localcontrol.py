import logging
import os
import subprocess
import multiprocessing


class Local:
    """
    A class that provides the controls for local multi-processing via the
    subprocess-module. It provides two methods:
    - is_job_finished(job): Returns True or False, depending on whether the job
        has finished execution
    - execute(job): Takes a job and executes it by spawning a new process for it
    """

    def __init__(self, max_number_of_processes=None):
        """!
        The default constructor.
        - Initializes a list which holds a connection between Script-Objects
          and their respective processes.
        - Initialized a logger which writes to validate_basf2.py's log.
        """

        ## A dict which holds the connections between the jobs (=Script objects)
        # and the processes that were spawned for them
        self.jobs_processes = {}

        ## Contains a reference to the logger-object from validate_basf2
        # Set up the logging functionality for the 'local execution'-Class,
        # so we can log to validate_basf2.py's log what is going on in
        # .execute and .is_finished
        self.logger = logging.getLogger('validate_basf2')

        ## Parameter for maximal number of parallel processes, use system CPU count if not specified
        # use the default of 10 if the cpu_count call is not supported on current platform
        try:
            self.max_number_of_processes = multiprocessing.cpu_count() if max_number_of_processes is None else max_number_of_processes
        except NotImplementedError:
            self.max_number_of_processes = 10

        self.logger.note("Executing validation with " + str(self.max_number_of_processes) + " parallel processes")

        ## Counter for number of running parallel processes
        self.current_number_of_processes = 0

    def available(self):
        """!
        Checks whether the number of current parallel processes is below the
        limit and a new process can be started.
        @return: True if a new process can be spawned, otherwise False
        """

        return (self.max_number_of_processes > 0) and (self.current_number_of_processes < self.max_number_of_processes)

    def execute(self, job, options=''):
        """!
        Takes a Script object and a string with options and runs it locally,
        either with ROOT or with basf2, depending on the file type.

        @param job: The steering file object that should be executed
        @param options: Options that will be given to the basf2 command
        @return: None
        """

        # Remember current working directory (to make sure the cwd is the same
        # after this function has finished)
        current_cwd = os.getcwd()

        # Define the folder in which the results (= the ROOT files) should be
        # created. This is where the files containing plots will end up. By
        # convention, data files will be stored in the parent dir.
        # Then make sure the folder exists (create if it does not exist) and
        # change to cwd to this folder.
        output_dir = './results/current/' + job.package
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        os.chdir(output_dir)

        # Create a logfile for this job and make sure it's empty!
        log = open(os.path.basename(job.path) + '.log', 'w+')

        # Now we need to distinguish between .py and .C files:
        extension = os.path.splitext(job.path)[1]
        if extension == '.C':
            # .c files are executed with ROOT. No options available here.
            params = ['root', '-b', '-q', job.path]
        else:
            # .py files are executed with basf2.
            # 'options' contains an option-string for basf2, e.g. '-n 100 -p
            # 8'. This string will be split on white-spaces and added to the
            # params-list, since subprocess.Popen does not like strings...
            params = ['basf2', job.path] + options.split()

        # Log the command we are about the execute
        self.logger.debug(subprocess.list2cmdline(params))

        # Spawn that new process which executes the command we just defined.
        # Output of it will be written to the file defined above ('log').
        process = subprocess.Popen(params, stdout=log, stderr=log)

        # Save the connection between the job and the given process-ID
        self.jobs_processes[job] = process

        # Increase the process counter
        self.current_number_of_processes += 1

        # Return to previous cwd
        os.chdir(current_cwd)

    def is_job_finished(self, job):
        """!
        Checks if a given job has finished.

        @param job: The job of which we want to know if it finished
        @return: True if the job has finished, otherwise False
        """

        # Look which process belongs to the given job
        process = self.jobs_processes[job]

        # Check if the process has finished or not, and return that together
        # with the return code / exit_status of the process.
        if process.poll() is not None:
            del self.jobs_processes[job]
            self.current_number_of_processes = len(self.jobs_processes.keys())
            return [True, process.returncode]
        else:
            return [False, 0]
