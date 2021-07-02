#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# std
import logging
import os
import stat
from abc import abstractmethod

# ours
from validationscript import Script


class ClusterBase:
    """
    Base class which provides basic functionality to wrap basf2 into a shell
    script setting up the environment and checking for completion of script
    """

    def __init__(self):
        """!
        The default constructor.
        """
        #: The path, where the help files are being created
        # Maybe there should be a special subfolder for them?
        self.path = os.getcwd()

        #: Contains a reference to the logger-object from validate_basf2
        # Set up the logging functionality for the 'cluster execution'-Class,
        # so we can log to validate_basf2.py's log what is going on in
        # .execute and .is_finished
        self.logger = logging.getLogger("validate_basf2")

        # We need to set up the same environment on the cluster like on the
        # local machine. The information can be extracted from $BELLE2_TOOLS,
        # $BELLE2_RELEASE_DIR and $BELLE2_LOCAL_DIR

        #: Path to the basf2 tools and central/local release
        self.tools = self.adjust_path(os.environ["BELLE2_TOOLS"])
        belle2_release_dir = os.environ.get("BELLE2_RELEASE_DIR", None)
        belle2_local_dir = os.environ.get("BELLE2_LOCAL_DIR", None)

        #: The command for b2setup (and b2code-option)
        self.b2setup = "b2setup"
        if belle2_release_dir is not None:
            self.b2setup += " " + belle2_release_dir.split("/")[-1]
        if belle2_local_dir is not None:
            self.b2setup = (
                "MY_BELLE2_DIR="
                + self.adjust_path(belle2_local_dir)
                + " "
                + self.b2setup
            )
        if os.environ.get("BELLE2_OPTION") != "debug":
            self.b2setup += "; b2code-option " + os.environ.get("BELLE2_OPTION")

        # Write to log which revision we are using
        self.logger.debug(f"Setting up the following release: {self.b2setup}")

        # Define the folder in which the log of the cluster messages will be
        # stored (same folder like the log for validate_basf2.py)
        clusterlog_dir = "./html/logs/__general__/"
        if not os.path.exists(clusterlog_dir):
            os.makedirs(clusterlog_dir)

        #: The file object to which all cluster messages will be written
        self.clusterlog = open(clusterlog_dir + "clusterlog.log", "w+")

    def createDoneFileName(self, job: Script) -> str:
        """!
        Generate the file name used for the done output
        """
        return f"{self.path}/script_{job.name}.done"

    def prepareSubmission(self, job: Script, options, tag):
        """!
        Setup output folders and create the wrapping shell script. Will return
        the full file name of the generated wrapper script.
        """

        # Define the folder in which the results (= the ROOT files) should be
        # created. This is where the files containing plots will end up. By
        # convention, data files will be stored in the parent dir.
        # Then make sure the folder exists (create if it does not exist) and
        # change to cwd to this folder.
        output_dir = os.path.abspath(f"./results/{tag}/{job.package}")
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        # Path where log file is supposed to be created
        # log_file = output_dir + '/' + os.path.basename(job.path) + '.log'

        # Remove any left over done files
        donefile_path = self.createDoneFileName(job)
        if os.path.isfile(donefile_path):
            os.remove(donefile_path)

        # Now we need to distinguish between .py and .C files:
        extension = os.path.splitext(job.path)[1]
        if extension == ".C":
            # .c files are executed with root
            command = "root -b -q " + job.path
        else:
            # .py files are executed with basf2
            # 'options' contains an option-string for basf2, e.g. '-n 100'
            command = f"basf2 {job.path} {options}"

        # Create a helpfile-shellscript, which contains all the commands that
        # need to be executed by the cluster.
        # First, set up the basf2 tools and perform b2setup with the correct
        # revision. The execute the command (i.e. run basf2 or ROOT on a
        # steering file). Write the return code of that into a *.done file.
        # Delete the helpfile-shellscript.
        tmp_name = self.path + "/" + "script_" + job.name + ".sh"
        with open(tmp_name, "w+") as tmp_file:
            tmp_file.write(
                "#!/bin/bash \n\n"
                + "BELLE2_NO_TOOLS_CHECK=1 \n"
                + f"source {self.tools}/b2setup \n"
                + "cd {} \n".format(self.adjust_path(output_dir))
                + f"{command} \n"
                + "echo $? > {}/script_{}.done \n".format(self.path, job.name)
                + f"rm {tmp_name} \n"
            )

        # Make the helpfile-shellscript executable
        st = os.stat(tmp_name)
        os.chmod(tmp_name, st.st_mode | stat.S_IEXEC)

        return tmp_name

    def checkDoneFile(self, job):
        """!
        Checks whether the '.done'-file has been created for a job. If so, it
        returns True, else it returns False in the first part of the tuple.
        Also deletes the .done-File it if exists.
        The second entry in the tuple will be the exit code read from the done file
        """

        # If there is a file indicating the job is done, that is its name:
        donefile_path = self.createDoneFileName(job)

        donefile_exists = False
        # Check if such a file exists. If so, this means that the job has
        # finished.
        if os.path.isfile(donefile_path):

            # Read the returncode/exit_status for the job from the *.done-file
            with open(donefile_path) as f:
                try:
                    returncode = int(f.read().strip())
                except ValueError:
                    returncode = -654

            print(f"donefile found with return code {returncode}")
            donefile_exists = True
            os.remove(donefile_path)
        else:
            print("no donefile found")
            returncode = -555

        return [donefile_exists, returncode]

    def terminate(self, job: Script):
        """! Terminate running job.
        """
        self.logger.error("Script termination not supported.")

    @abstractmethod
    def adjust_path(self, path):
        """!
        This method can be used if path names are different on submission
        and execution hosts.
        @param path: The past that needs to be adjusted
        @return: The adjusted path
        """
