import logging
import os
import subprocess


class Cluster:
    """
    A class that provides the controls for running jobs on a (remote)
    cluster. It provides two methods:
    - is_job_finished(job): Returns True or False, depending on whether the job
        has finished execution
    - execute(job): Takes a job and executes it by sending it to the cluster
    """

    def __init__(self):
        """
        The default constructor.
        - Holds the current working directory, which is also the location of
          the shellscripts that are being sent to the cluster.
        - Initializes a logger which writes to validate_basf2.py's log.
        - Finds the revision of basf2 that will be set up on the cluster.
        """

        # The command to submit a job. 'LOGFILE' will be replaced by the
        # actual log file name
        self.submit_command = 'qsub -cwd -o LOGFILE -e LOGFILE -q medium -V'

        # The path, where the help files are being created
        # Maybe there should be a special subfolder for them?
        self.path = os.getcwd()

        # Set up the logging functionality for the 'cluster execution'-Class,
        # so we can log to validate_basf2.py's log what is going on in
        # .execute and .is_finished
        self.logger = logging.getLogger('validate_basf2.clustercontrol')

        # We need to set up the same environment on the cluster like on the
        # local machine. The information can be extracted from $BELLE2_TOOLS,
        # $BELLE2_RELEASE_DIR and $BELLE2_LOCAL_DIR
        self.tools = self.adjust_path(os.environ['BELLE2_TOOLS'])
        belle2_release_dir = os.environ.get('BELLE2_RELEASE_DIR', None)
        belle2_local_dir = os.environ.get('BELLE2_LOCAL_DIR', None)
        self.setuprel = 'setuprel'
        if belle2_release_dir is not None:
            self.setuprel += ' ' + belle2_release_dir.split('/')[-1]
        if belle2_local_dir is not None:
            self.setuprel = 'MY_BELLE2_DIR=' + self.adjust_path(belle2_local_dir) + ' ' + self.setuprel

        # Write to log which revision we are using
        self.logger.debug('Setting up the following release: {0}'
                          .format(self.setuprel))

        # Define the folder in which the log of the cluster messages will be
        # stored (same folder like the log for validate_basf2.py)
        clusterlog_dir = './html/logs/__general__/'
        if not os.path.exists(clusterlog_dir):
            os.makedirs(clusterlog_dir)
        self.clusterlog = open(clusterlog_dir + 'clusterlog.log', 'w+')

    def adjust_path(self, path):
        """
        This method can be used if path names are different on submission
        and execution hosts.
        """

        return path

    def available(self):
        """
        The cluster should always be available to accept new jobs.
        """

        return True

    def execute(self, job, options=''):
        """
        Takes a Script object and a string with options and runs it on the
        cluster, either with ROOT or with basf2, depending on the file type.
        """

        # Define the folder in which the results (= the ROOT files) should be
        # created. This is where the files containing plots will end up. By
        # convention, data files will be stored in the parent dir.
        # Then make sure the folder exists (create if it does not exist) and
        # change to cwd to this folder.
        output_dir = os.path.abspath('./results/current/' + job.package)
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        # Path where log file is supposed to be created
        log_file = output_dir + '/' + job.name + '.log'

        # Now we need to distinguish between .py and .C files:
        extension = os.path.splitext(job.path)[1]
        if extension == '.C':
            # .c files are executed with root
            command = 'root -b -q ' + job.path
        else:
            # .py files are executed with basf2
            # 'options' contains an option-string for basf2, e.g. '-n 100'
            command = 'basf2 {0} {1}'.format(job.path, options)

        # Create a helpfile-shellscript, which contains all the commands that
        # need to be executed by the cluster.
        # First, set up the basf2 tools and perform setuprel with the correct
        # revision. The execute the command (i.e. run basf2 or ROOT on a
        # steering file). Write the return code of that into a *.done file.
        # Delete the helpfile-shellscript.
        tmp_name = self.path + '/' + 'script_' + job.name + '.sh'
        with open(tmp_name, 'w+') as tmp_file:
            tmp_file.write('#!/bin/bash \n\n' +
                           'BELLE2_NO_TOOLS_CHECK=1 \n' +
                           'source {0}/setup_belle2 \n'.format(self.tools) +
                           '{0} \n'.format(self.setuprel) +
                           'cd {0} \n'.format(self.adjust_path(output_dir)) +
                           '{0} \n'.format(command) +
                           'echo $? > {0}/script_{1}.done \n'
                           .format(self.path, job.name) +
                           'rm {0} \n'.format(tmp_name))

        # Prepare the command line command for submission to the cluster
        params = self.submit_command.replace('LOGFILE', log_file).split() + [tmp_name]

        # Log the command we are about the execute
        self.logger.debug(subprocess.list2cmdline(params))

        # Submit it to the cluster and throw away the cluster output (which is
        # usually only "Your job [Job ID] has been submitted"). The steering
        # file output will be written to 'log_file' (see above).
        subprocess.Popen(params, stdout=self.clusterlog, stderr=self.clusterlog)

    def is_job_finished(self, job):
        """
        Checks whether the '.done'-file has been created for a job. If so, it
        returns True, else it returns False.
        Also deletes the .done-File once it has returned True.
        """

        # If there is a file indicating the job is done, that is its name:
        donefile_path = "{0}/script_{1}.done".format(self.path, job.name)

        # Check if such a file exists. If so, this means that the job has
        # finished.
        if os.path.isfile(donefile_path):

            # Read the returncode / exit_status for the job from the *.done-file
            with open(donefile_path) as f:
                try:
                    returncode = int(f.read().strip())
                except ValueError:
                    returncode = -666

            # Delete the *.done file
            os.remove(donefile_path)

            #Return that the job is finished + the return code for it
            return [True, returncode]

        # If no such file exists, the job has not yet finished
        else:
            return [False, 0]
