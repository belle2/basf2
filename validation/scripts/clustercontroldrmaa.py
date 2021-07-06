#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# std
import sys

# ours
from clustercontrolbase import ClusterBase
from validationscript import Script


class Cluster(ClusterBase):
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
        try:
            import drmaa  # noqa

            return True
        except ImportError:
            print(
                "drmaa library is not installed, please ues 'pip3 install "
                "drmaa'"
            )
            return False
        except RuntimeError as re:
            print("drmaa library not properly configured")
            print(str(re))
            return False

    @staticmethod
    def name():
        """
        Returns name of this job contol
        """
        return "cluster-drmaa"

    @staticmethod
    def description():
        """
        Returns description of this job control
        """
        return "Batch submission via the drmaa interface to Grid Engine"

    def __init__(self):
        """!
        The default constructor.
        - Holds the current working directory, which is also the location of
          the shellscripts that are being sent to the cluster.
        - Initializes a logger which writes to validate_basf2.py's log.
        - Finds the revision of basf2 that will be set up on the cluster.
        """

        #: The command to submit a job. 'LOGFILE' will be replaced by the
        #: actual log file name
        self.native_spec = (
            "-l h_vmem={requirement_vmem}G,h_fsize={"
            "requirement_storage}G "
            "-q {queuename} -V"
        )

        #: required vmem by the job in GB, required on DESY NAF, otherwise
        #: jobs get killed due to memory consumption
        self.requirement_vmem = 4

        #: the storage IO in GB which can be performed by each job. By
        #: default, this is 3GB at DESY which is to small for some validation
        #:  scripts
        self.requirement_storage = 50

        #: Queue best suitable for execution at DESY NAF
        self.queuename = "short.q"

        # call the base constructor, which will setup the batch cluster
        # common stuff
        super().__init__()

    # noinspection PyMethodMayBeStatic
    def adjust_path(self, path: str):
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

    def execute(self, job: Script, options="", dry=False, tag="current"):
        """!
        Takes a Script object and a string with options and runs it on the
        cluster, either with ROOT or with basf2, depending on the file type.

        @param job: The steering file object that should be executed
        @param options: Options that will be given to the basf2 command
        @param dry: Whether to perform a dry run or not
        @param tag: The folder within the results directory
        @return: None
        """

        # import here first so the whole module can also be imported on python
        # installations which have no drmaa at all
        import drmaa

        print(str(drmaa.Session()))

        with drmaa.Session() as session:
            print("got session ")
            print(str(session))

            shell_script_name = self.prepareSubmission(job, options, tag)

            # native specification with all the good settings for the batch
            # server
            native_spec_string = self.native_spec.format(
                requirement_storage=self.requirement_storage,
                requirement_vmem=self.requirement_vmem,
                queuename=self.queuename,
            )
            print(
                f"Creating job template for wrapper script {shell_script_name}"
            )
            jt = session.createJobTemplate()
            jt.remoteCommand = shell_script_name
            jt.joinFiles = True
            jt.nativeSpecification = native_spec_string

            if not dry:
                jobid = session.runJob(jt)
                self.logger.debug(
                    f"Script {job.name} started with job id {jobid}"
                )
                job.job_id = jobid

            session.deleteJobTemplate(jt)
        return

    def is_job_finished(self, job: Script):
        """!
        Checks whether the '.done'-file has been created for a job. If so, it
        returns True, else it returns False.
        Also deletes the .done-File once it has returned True.

        @param job: The job of which we want to know if it finished
        @return: (True if the job has finished, exit code). If we can't find the
            exit code in the '.done'-file, the returncode will be -654.
            If the job is not finished, the exit code is returned as 0.
        """

        # import here first so the whole module can also be imported on python
        # installations which have no drmaa at all
        import drmaa

        if job.job_id is None:
            print(
                "Job has not been started with cluster drmaaa because "
                "job id is missing"
            )
            sys.exit(0)

        with drmaa.Session() as session:

            # some batch server will forget completed jobs right away
            try:
                status = session.jobStatus(job.job_id)
            except drmaa.errors.InvalidJobException:
                print(
                    "Job info for jobid {} cannot be retrieved, assuming "
                    "job has terminated".format(job.job_id)
                )

                (donefile_exists, donefile_returncode) = self.checkDoneFile(job)

                # always return the job es complete even if there is no done
                #  file at this ponint tho job is also not longer
                # running/queued on the cluster
                return [True, donefile_returncode]

            # Return that the job is finished + the return code for it
            # depending when we look for the job this migh never be used,
            # because the jobs disappear from qstat before we can query them
            #  ..
            if status == drmaa.JobState.DONE:
                # todo: return code
                return [True, 0]
            if status == drmaa.JobState.FAILED:
                return [True, 1]

            return [False, 0]
