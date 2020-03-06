#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# std
import logging
import os
import subprocess
import stat
import shutil
import re
from typing import Tuple

# ours
from validationscript import Script


class Cluster:
    """
    A class that provides the controls for running jobs on a (remote)
    cluster. It provides two methods:
    - is_job_finished(job): Returns True or False, depending on whether the job
        has finished execution
    - execute(job): Takes a job and executes it by sending it to the cluster
    """

    @staticmethod
    def is_supported():
        """
        Check if the bsub command is available
        """
        return shutil.which("bsub") is not None

    @staticmethod
    def name():
        """
        Returns name of this job contol
        """
        return "cluster"

    @staticmethod
    def description():
        """
        Returns description of this job control
        """
        return "Batch submission to bsub-based cluster"

    def __init__(self):
        """!
        The default constructor.
        - Holds the current working directory, which is also the location of
          the shellscripts that are being sent to the cluster.
        - Initializes a logger which writes to validate_basf2.py's log.
        - Finds the revision of basf2 that will be set up on the cluster.
        """

        #: The path, where the help files are being created
        #: Maybe there should be a special subfolder for them?
        self.path = os.getcwd()

        #: Contains a reference to the logger-object from validate_basf2
        #: Set up the logging functionality for the 'cluster execution'-Class,
        #: so we can log to validate_basf2.py's log what is going on in
        #: .execute and .is_finished
        self.logger = logging.getLogger('validate_basf2')

        # We need to set up the same environment on the cluster like on the
        # local machine. The information can be extracted from $BELLE2_TOOLS,
        # $BELLE2_RELEASE_DIR and $BELLE2_LOCAL_DIR

        #: Path to the basf2 tools and central/local release
        self.tools = self.adjust_path(os.environ['BELLE2_TOOLS'])
        belle2_release_dir = os.environ.get('BELLE2_RELEASE_DIR', None)
        belle2_local_dir = os.environ.get('BELLE2_LOCAL_DIR', None)

        #: The command for b2setup (and b2code-option)
        self.b2setup = 'b2setup'
        if belle2_release_dir is not None:
            self.b2setup += ' ' + belle2_release_dir.split('/')[-1]
        if belle2_local_dir is not None:
            self.b2setup = 'MY_BELLE2_DIR=' + \
                self.adjust_path(belle2_local_dir) + ' ' + self.b2setup
        if os.environ.get('BELLE2_OPTION') != 'debug':
            self.b2setup += '; b2code-option ' + \
                            os.environ.get('BELLE2_OPTION')

        # Write to log which revision we are using
        self.logger.debug(f'Setting up the following release: {self.b2setup}')

        # Define the folder in which the log of the cluster messages will be
        # stored
        clusterlog_dir = './html/logs/__general__/'
        if not os.path.exists(clusterlog_dir):
            os.makedirs(clusterlog_dir)

        # fixme: When will this be closed?
        #: The file object to which all cluster messages will be written
        #: Opened once for all job submissions
        self.clusterlog = open(clusterlog_dir + 'clusterlog.log', 'w+')

    # noinspection PyMethodMayBeStatic
    def adjust_path(self, path):
        """!
        This method can be used if path names are different on submission
        and execution hosts.
        @param path: The past that needs to be adjusted
        @return: The adjusted path
        """

        return path

    # noinspection PyMethodMayBeStatic
    def available(self):
        """!
        The cluster should always be available to accept new jobs.
        @return: Will always return True if the function can be called
        """

        return True

    def execute(self, job: Script, options='', dry=False, tag='current'):
        """!
        Takes a Script object and a string with options and runs it on the
        cluster, either with ROOT or with basf2, depending on the file type.

        @param job: The steering file object that should be executed
        @param options: Options that will be given to the basf2 command
        @param dry: Whether to perform a dry run or not
        @param tag: The folder within the results directory
        @return: None
        """

        # Define the folder in which the results (= the ROOT files) should be
        # created. This is where the files containing plots will end up. By
        # convention, data files will be stored in the parent dir.
        # Then make sure the folder exists (create if it does not exist) and
        # change to cwd to this folder.
        output_dir = os.path.abspath(f'./results/{tag}/{job.package}')
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        log_file = output_dir + '/' + os.path.basename(job.path) + '.log'

        # Remove any left over done files
        donefile_path = f"{self.path}/script_{job.name}.done"
        if os.path.isfile(donefile_path):
            os.remove(donefile_path)

        extension = os.path.splitext(job.path)[1]
        if extension == '.C':
            # .c files are executed with root
            command = 'root -b -q ' + job.path
        else:
            # .py files are executed with basf2
            # 'options' contains an option-string for basf2, e.g. '-n 100'
            command = f'basf2 {job.path} {options}'

        # Create a helpfile-shellscript, which contains all the commands that
        # need to be executed by the cluster.
        # First, set up the basf2 tools and perform b2setup with the correct
        # revision. The execute the command (i.e. run basf2 or ROOT on a
        # steering file). Write the return code of that into a *.done file.
        # Delete the helpfile-shellscript.
        tmp_name = self._get_tmp_name(job)
        with open(tmp_name, 'w+') as tmp_file:
            tmp_file.write('#!/bin/bash \n\n' +
                           'BELLE2_NO_TOOLS_CHECK=1 \n' +
                           'source {0}/b2setup \n'.format(self.tools) +
                           'cd {0} \n'.format(self.adjust_path(output_dir)) +
                           '{0} \n'.format(command) +
                           'echo $? > {0}/script_{1}.done \n'
                           .format(self.path, job.name) +
                           'rm {0} \n'.format(tmp_name))

        # Make the helpfile-shellscript executable
        st = os.stat(tmp_name)
        os.chmod(tmp_name, st.st_mode | stat.S_IEXEC)

        # Prepare the command line command for submission to the cluster
        params = [
            "bsub", "-o", log_file, "-e", log_file, "-q", "l", tmp_name,
        ]

        # Log the command we are about the execute
        self.logger.debug(subprocess.list2cmdline(params))

        if not dry:
            # Submit job
            process = subprocess.Popen(
                params, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                universal_newlines=True
            )
            output, error = process.communicate()
            self.clusterlog.writelines([output, error])

            if process.wait() != 0:
                # Submission did not succeed
                job.status = 'failed'
                self._cleanup(job)
            else:
                # Submission succeeded. Get Job ID by parsing output, so that
                # we can terminate the job later.
                res = re.search(output, "Job <([0-9]*)> is submitted")
                if res:
                    job.job_id = res.group(1)
                else:
                    self.logger.error(
                        "Could not find job id! Will not be able to terminate"
                        " this job, even if necessary."
                    )
        else:
            os.system(f'echo 0 > {self.path}/script_{job.name}.done')
            self._cleanup(job)

    def _cleanup(self, job: Script) -> None:
        """ Clean up after job has finished. """
        tmp_name = self._get_tmp_name(job)
        os.system(f'rm {tmp_name}')

    def _get_tmp_name(self, job: Script) -> str:
        """ Name of temporary file used for job submission. """
        return self.path + '/' + 'script_' + job.name + '.sh'

    def is_job_finished(self, job: Script) -> Tuple[bool, int]:
        """!
        Checks whether the '.done'-file has been created for a job. If so, it
        returns True, else it returns False.
        Also deletes the .done-File once it has returned True.

        @param job: The job of which we want to know if it finished
        @return: (True if the job has finished, exit code). If we can't find the
            exit code in the '.done'-file, the returncode will be -666.
            If the job is not finished, the exit code is returned as 0.
        """

        donefile_path = f"{self.path}/script_{job.name}.done"

        if os.path.isfile(donefile_path):
            # Job finished.
            # Read the returncode/exit_status
            with open(donefile_path) as f:
                try:
                    returncode = int(f.read().strip())
                except ValueError:
                    returncode = -666

            os.remove(donefile_path)

            return True, returncode

        else:
            # If no such file exists, the job has not yet finished
            return False, 0

    def terminate(self, job: Script):
        """! Terminate a running job
        """
        if job.job_id:
            params = ["bkill", job.job_id]
            self.logger.debug(subprocess.list2cmdline(params))

            process = subprocess.Popen(
                params,
                stdout=self.clusterlog,
                stderr=subprocess.STDOUT
            )
            process.wait()
            self._cleanup(job)
        else:
            self.logger.error(
                "Termination of the job corresponding to steering file "
                f"{job.path} has been requested, but no job id is available."
                f" Can't do anything.")
