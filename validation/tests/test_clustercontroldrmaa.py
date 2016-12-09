#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import sys
import tempfile
import os
from enum import Enum
from unittest.mock import MagicMock
from clustercontroldrmaa import Cluster
from validationscript import Script


class TestClusterControlDrmaa(unittest.TestCase):

    class SessionMock():

        class JobState(Enum):
            DONE = 1
            FAILED = 2
            RUNNING = 3

        jobStatusReturn = JobState.RUNNING

        def __enter__(self):
            return self

        def __exit__(self, exc, value, tb):
            pass

        def createJobTemplate(self):
            return MagicMock()

        def runJob(self, jt):
            return MagicMock()

        def jobStatus(self, jt):
            return self.jobStatusReturn

        def deleteJobTemplate(self, jt):
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
            self.assertTrue(job.cluster_drmaa_jobid)

            # check on job, not finished yet
            self.assertFalse(cc.is_job_finished(job)[0])

            # check on job, finished !
            # change behaviour of jobStatus
            self.SessionMock.jobStatusReturn = self.SessionMock.JobState.DONE

            self.assertTrue(cc.is_job_finished(job)[0])

            cc.terminate()

if __name__ == "__main__":
    unittest.main()
