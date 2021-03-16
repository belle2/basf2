#!/usr/bin/env python3

import unittest
import sys
import tempfile
import os
from enum import Enum
from unittest.mock import MagicMock
from clustercontroldrmaa import Cluster
from validationscript import Script


class TestClusterControlDrmaa(unittest.TestCase):
    """
    Test for for the DRMAA-backend of the clustercontrol
    """
    class SessionMock:
        """
        Class to mock a DRMAA session
        """

        class JobState(Enum):
            """
            Possible DRMAA Job States
            """
            #: job done
            DONE = 1
            #: job failed
            FAILED = 2
            #: job running
            RUNNING = 3

        #: the job state this session mock will return
        jobStatusReturn = JobState.RUNNING

        def __enter__(self):
            """ to support python with syntax"""
            return self

        def __exit__(self, exc, value, tb):
            """ to support python with syntax"""

        def createJobTemplate(self):
            """fake creating job template"""
            return MagicMock()

        def runJob(self, jt):
            """fake job running"""
            return MagicMock()

        def jobStatus(self, jt):
            """fake job status terun"""
            return self.jobStatusReturn

        def deleteJobTemplate(self, jt):
            """fake job deletion"""
            return MagicMock()

    def test_no_drmaa(self):
        """
        Overwrite drmaa module and see what happens ...
        """
        sys.modules['drmaa'] = None
        self.assertFalse(Cluster.is_supported())

    def test_submit_and_monitor_job(self):
        """
        Test to submit and monitor a regular job
        """

        with tempfile.TemporaryDirectory() as td:
            # switch to temp folder as the cluster controller will create
            # some helper files
            os.chdir(str(td))

            drmaa_mock = MagicMock()
            drmaa_mock.Session = self.SessionMock  # MagicMock(return_value=session_mock)
            drmaa_mock.JobState = self.SessionMock.JobState

            sys.modules['drmaa'] = drmaa_mock

            # this just imports the drmaa module which should always work with our
            # mock
            self.assertTrue(Cluster.is_supported())

            job = Script(path="myscript1.py", package="test_package", log=None)

            cc = Cluster()
            cc.execute(job)

            # check if job id has been set
            self.assertTrue(job.job_id)

            # check on job, not finished yet
            self.assertFalse(cc.is_job_finished(job)[0])

            # check on job, finished !
            # change behaviour of jobStatus
            self.SessionMock.jobStatusReturn = self.SessionMock.JobState.DONE

            self.assertTrue(cc.is_job_finished(job)[0])

            cc.terminate(job)


if __name__ == "__main__":
    unittest.main()
