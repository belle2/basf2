#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import os
import subprocess
import stat
import shutil
import validationfunctions


class Cluster:
    """
    A class that provides the controls for running jobs on a (remote)
    Sun Grid Engine cluster. It provides two methods:
    - is_job_finished(job): Returns True or False, depending on whether the job
        has finished execution
    - execute(job): Takes a job and executes it by sending it to the cluster
    """

    @staticmethod
    def is_supported():
        """
        Check if qsub is available
        """
        return shutil.which("qsub") is not None

    @staticmethod
    def name():
        """
        Returns name of this job contol
        """
        return "cluster-sge"

    @staticmethod
    def description():
        """
        Returns description of this job control
        """
        return "Batch submission via command line to Grid Engine"

    def __init__(self):
        """!
        The default constructor.
        - Holds the current working directory, which is also the location of
          the shellscripts that are being sent to the cluster.
        - Initializes a logger which writes to validate_basf2.py's log.
        - Finds the revision of basf2 that will be set up on the cluster.
        """

        #: The command to submit a job. 'LOGFILE' will be replaced by the
        # actual log file name
        self.submit_command = ('qsub -cwd -l h_vmem={requirement_vmem}G,h_fsize={requirement_storage}G '
                               '-o {logfile} -e {logfile} -q {queuename} -V')

        #: required vmem by the job in GB, required on DESY NAF, otherwise jobs get killed due
        # to memory consumption
        self.requirement_vmem = 4

        #: the storage IO in GB which can be performed by each job. By default, this is 3GB at
        # DESY which is to small for some validation scripts
        self.requirement_storage = 50

        #: Queue best suitable for execution at DESY NAF
        self.queuename = "short.q"

        #: The path, where the help files are being created
        # Maybe there should be a special subfolder for them?
        self.path = os.getcwd()

        #: Contains a reference to the logger-object from validate_basf2
        # Set up the logging functionality for the 'cluster execution'-Class,
        # so we can log to validate_basf2.py's log what is going on in
        # .execute and .is_finished
        self.logger = logging.getLogger('validate_basf2')

        # We need to set up the same environment on the cluster like on the
        # local machine. The information can be extracted from $BELLE2_TOOLS,
        # $BELLE2_RELEASE_DIR and $BELLE2_LOCAL_DIR

        #: Path to the basf2 tools and central/local release
        self.tools = self.adjust_path(os.environ['BELLE2_TOOLS'])
        belle2_release_dir = os.environ.get('BELLE2_RELEASE_DIR', None)
        belle2_local_dir = os.environ.get('BELLE2_LOCAL_DIR', None)

        #: The command for b2setup (and setoption)
        self.b2setup = 'b2setup'
        if belle2_release_dir is not None:
            self.b2setup += ' ' + belle2_release_dir.split('/')[-1]
        if belle2_local_dir is not None:
            self.b2setup = 'MY_BELLE2_DIR=' + \
                self.adjust_path(belle2_local_dir) + ' ' + self.b2setup
        if os.environ.get('BELLE2_OPTION') != 'debug':
            self.b2setup += '; b2code-option ' + os.environ.get('BELLE2_OPTION')

        # Write to log which revision we are using
        self.logger.debug('Setting up the following release: {0}'
                          .format(self.b2setup))

        # Define the folder in which the log of the cluster messages will be
        # stored (same folder like the log for validate_basf2.py)
        clusterlog_dir = './html/logs/__general__/'
        if not os.path.exists(clusterlog_dir):
            os.makedirs(clusterlog_dir)

        #: The file object to which all cluster messages will be written
        self.clusterlog = open(clusterlog_dir + 'clusterlog.log', 'w+')

    def adjust_path(self, path):
        """!
        This method can be used if path names are different on submission
        and execution hosts.
        @param path: The past that needs to be adjusted
        @return: The adjusted path
        """

        return path

    def available(self):
        """!
        The cluster should always be available to accept new jobs.
        @return: Will always return True if the function can be called
        """

        return True

    def execute(self, job, options='', dry=False, tag='current'):
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
        output_dir = os.path.abspath('./results/{0}/{1}'.
                                     format(tag, job.package))
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        # Path where log file is supposed to be created
        log_file = output_dir + '/' + os.path.basename(job.path) + '.log'

        # Remove any left over done files
        donefile_path = "{0}/script_{1}.done".format(self.path, job.name)
        if os.path.isfile(donefile_path):
            os.remove(donefile_path)

        # Now we need to distinguish between .py and .C files:
        extension = os.path.splitext(job.path)[1]
        if extension == '.C':
            # .c files are executed with root
            command = 'root -b -q ' + job.path
        else:
            # .py files are executed with basf2
            # 'options' contains an option-string for basf2, e.g. '-n 100'
            params = validationfunctions.basf2_command_builder(job.path, options.split())
            command = subprocess.list2cmdline(params)

        # Create a helpfile-shellscript, which contains all the commands that
        # need to be executed by the cluster.
        # First, set up the basf2 tools and perform b2setup with the correct
        # revision. The execute the command (i.e. run basf2 or ROOT on a
        # steering file). Write the return code of that into a *.done file.
        # Delete the helpfile-shellscript.
        tmp_name = self.path + '/' + 'script_' + job.name + '.sh'
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
        params = self.submit_command.format(queuename=self.queuename,
                                            requirement_storage=self.requirement_storage,
                                            requirement_vmem=self.requirement_vmem,
                                            logfile=log_file).split() + [tmp_name]

        # Log the command we are about the execute
        self.logger.debug(subprocess.list2cmdline(params))

        # Submit it to the cluster. The steering
        # file output will be written to 'log_file' (see above).
        # If we are performing a dry run, don't send anything to the cluster
        # and just create the *.done file right away and delete the *.sh file.
        if not dry:
            process = subprocess.Popen(params, stdout=self.clusterlog,
                                       stderr=subprocess.STDOUT)

            # Check whether the submission succeeded
            if process.wait() != 0:
                job.status = 'failed'
        else:
            os.system('echo 0 > {0}/script_{1}.done'.format(self.path,
                                                            job.name))
            os.system('rm {0}'.format(tmp_name))

    def is_job_finished(self, job):
        """!
        Checks whether the '.done'-file has been created for a job. If so, it
        returns True, else it returns False.
        Also deletes the .done-File once it has returned True.

        @param job: The job of which we want to know if it finished
        @return: True if the job has finished, otherwise False
        """

        # If there is a file indicating the job is done, that is its name:
        donefile_path = "{0}/script_{1}.done".format(self.path, job.name)

        # Check if such a file exists. If so, this means that the job has
        # finished.
        if os.path.isfile(donefile_path):

            # Read the returncode/exit_status for the job from the *.done-file
            with open(donefile_path) as f:
                try:
                    returncode = int(f.read().strip())
                except ValueError:
                    returncode = -666

            # Delete the *.done file
            os.remove(donefile_path)

            # Return that the job is finished + the return code for it
            return [True, returncode]

        # If no such file exists, the job has not yet finished
        else:
            return [False, 0]

    def terminate(self, job):
        """!
        Terminate a running job, not support with this backend so ignore the call
        """
        pass
