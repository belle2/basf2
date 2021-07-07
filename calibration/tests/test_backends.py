#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Unit tests for the backends.py classes and functions. This is a little difficult to test due to
the Batch system backends not really being testable when not running on specific hosts with bsub/qsub
installed. But we can try to implement tests using the Local multiprocessing backend and testing the
basic behaviour of the classes so that they will fail if we modify assumptions in the future.
"""

from basf2 import find_file

import unittest
from unittest import TestCase
import shutil
from pathlib import Path

from caf.backends import ArgumentsSplitter, Job, MaxFilesSplitter, MaxSubjobsSplitter
from caf.backends import ArgumentsGenerator, range_arguments, SplitterError

# A testing directory so that we can do cleanup
test_dir = Path("test_backends").absolute()
# A simple bash script for testing jobs
test_script = Path(find_file("calibration/examples/job_submission/test_script.sh")).absolute()


class TestJob(TestCase):
    """
    UnitTest for the `caf.backends.Job` class
    """

    def setUp(self):
        """
        Create useful objects for each test and the teardown
        """
        # We will deliberately use strings and not Path objects for the job attributes so that we can later
        # check that they are converted to Paths by the class itself.
        name1 = 'TestJob1'
        job1 = Job(name1)  # Set up this one manually setting attributes
        job1.working_dir = Path(test_dir, job1.name, "working_dir").absolute().as_posix()
        job1.output_dir = Path(test_dir, job1.name, "output_dir").absolute().as_posix()
        job1.cmd = ["bash", test_script.name]
        job1.input_sandbox_files = [test_script.as_posix()]
        self.job1 = job1

        name2 = 'TestJob2'
        job_dict = {}
        job_dict["name"] = name2
        job_dict["working_dir"] = Path(test_dir, name2, "working_dir").as_posix()
        job_dict["output_dir"] = Path(test_dir, name2, "output_dir").as_posix()
        job_dict["output_patterns"] = []
        job_dict["cmd"] = ["bash", test_script.name]
        job_dict["args"] = []
        job_dict["input_sandbox_files"] = [test_script.as_posix()]
        job_dict["input_files"] = []
        job_dict["setup_cmds"] = []
        job_dict["backend_args"] = {}
        job_dict["subjobs"] = [{"id": i, "input_files": [], "args": [str(i)]} for i in range(4)]
        self.job2_dict = job_dict
        self.job2 = Job(name2, job_dict=job_dict)  # Set up this one from a dictionary

        # Create a directory just in case we need it for each test so that we can delete everything easily at the end
        test_dir.mkdir(parents=True, exist_ok=False)

    def test_dict_setup(self):
        self.maxDiff = None  # If this test fails you will need to see the diff of a large dictionary
        self.assertEqual(len(self.job2.subjobs), 4)
        self.assertEqual(self.job2_dict, self.job2.job_dict)
        self.job2_dict["subjobs"].pop()
        del self.job2.subjobs[3]
        self.assertEqual(self.job2_dict, self.job2.job_dict)

    def test_job_json_serialise(self):
        json_path = Path(test_dir, "job2.json")
        self.job2.dump_to_json(json_path)
        job2_copy = Job.from_json(json_path)
        self.assertEqual(self.job2.job_dict, job2_copy.job_dict)

    def test_status(self):
        """
        The Jobs haven't been run so they should be in the 'init' status.
        They also shouldn't throw exceptions due to missing result objects.
        """
        self.assertEqual(self.job1.status, "init")
        self.assertEqual(self.job2.status, "init")
        self.assertFalse(self.job1.ready())
        self.assertFalse(self.job2.ready())
        self.assertEqual(self.job1.update_status(), "init")
        self.assertEqual(self.job2.update_status(), "init")
        for subjob in self.job2.subjobs.values():
            self.assertEqual(subjob.status, "init")
            self.assertFalse(subjob.ready())
            self.assertEqual(subjob.update_status(), "init")

    def test_path_object_conversion(self):
        """
        Make sure that the two ways of setting up Job objects correctly converted attributes to be Paths instead of strings.
        """
        self.assertIsInstance(self.job1.output_dir, Path)
        self.assertIsInstance(self.job1.working_dir, Path)
        for path in self.job1.input_sandbox_files:
            self.assertIsInstance(path, Path)
        for path in self.job1.input_files:
            self.assertIsInstance(path, Path)

        self.assertIsInstance(self.job2.output_dir, Path)
        self.assertIsInstance(self.job2.working_dir, Path)
        for path in self.job2.input_sandbox_files:
            self.assertIsInstance(path, Path)
        for path in self.job2.input_files:
            self.assertIsInstance(path, Path)

        for subjob in self.job2.subjobs.values():
            self.assertIsInstance(subjob.output_dir, Path)
            self.assertIsInstance(subjob.working_dir, Path)

    def test_subjob_splitting(self):
        """
        Test the creation of SubJobs and assignment of input data files via splitter classes.
        """
        self.assertIsNone(self.job1.splitter)
        self.assertIsNone(self.job2.splitter)
        # Set the splitter for job1
        self.job1.max_files_per_subjob = 2
        self.assertIsInstance(self.job1.splitter, MaxFilesSplitter)
        self.assertEqual(self.job1.splitter.max_files_per_subjob, 2)
        self.job1.max_subjobs = 3
        self.assertIsInstance(self.job1.splitter, MaxSubjobsSplitter)
        self.assertEqual(self.job1.splitter.max_subjobs, 3)

        # Generate some empty input files
        for i in range(5):
            input_file = Path(test_dir, f"{i}.txt")
            input_file.touch(exist_ok=False)
            self.job1.input_files.append(input_file)

        self.job1.splitter = MaxFilesSplitter(max_files_per_subjob=2)
        self.job1.splitter.create_subjobs(self.job1)
        self.assertEqual(len(self.job1.subjobs), 3)  # Did the splitter create the number of jobs we expect?
        for i, subjob in self.job1.subjobs.items():
            self.assertTrue((len(subjob.input_files) == 2 or len(subjob.input_files) == 1))

        self.job1.subjobs = {}
        self.job1.splitter = MaxSubjobsSplitter(max_subjobs=4)
        self.job1.splitter.create_subjobs(self.job1)
        self.assertEqual(len(self.job1.subjobs), 4)  # Did the splitter create the number of jobs we expect?
        for i, subjob in self.job1.subjobs.items():
            self.assertTrue((len(subjob.input_files) == 2 or len(subjob.input_files) == 1))

        # Does the ArgumentSplitter create jobs
        self.job1.subjobs = {}
        arg_gen = ArgumentsGenerator(range_arguments, 3, stop=12, step=2)
        self.job1.splitter = ArgumentsSplitter(arguments_generator=arg_gen, max_subjobs=10)
        self.job1.splitter.create_subjobs(self.job1)
        self.assertEqual(len(self.job1.subjobs), 5)
        for (i, subjob), arg in zip(self.job1.subjobs.items(), range(3, 12, 2)):
            # Does each subjob receive the correct setup
            self.assertEqual(self.job1.input_files, subjob.input_files)
            self.assertEqual(arg, subjob.args[0])

        # Does max_jobs prevent infinite subjob numbers
        self.job1.subjobs = {}
        self.job1.splitter = ArgumentsSplitter(arguments_generator=arg_gen, max_subjobs=2)
        self.assertRaises(SplitterError, self.job1.splitter.create_subjobs, self.job1)

    def test_input_sandbox_copy(self):
        """
        Does the copy of files/directories for the input sandbox work correctly?
        """
        # We create a directory to add to the Job's input sandbox list, to tes if directories + contents are copied.
        input_sandbox_dir = Path(test_dir, "test_input_sandbox_dir")
        input_sandbox_dir.mkdir(parents=True, exist_ok=False)
        for i in range(2):
            input_file = Path(input_sandbox_dir, f"{i}.txt")
            input_file.touch(exist_ok=False)
        # Instead of identifying every file, we just use the whole directory
        self.job1.input_sandbox_files.append(input_sandbox_dir)
        # Manually create the working dir first (normally done by the Backend)
        self.job1.working_dir.mkdir(parents=True, exist_ok=False)
        self.job1.copy_input_sandbox_files_to_working_dir()

        # We expect the original script and the above extra files + parent directory to be copied to the working directory.
        expected_paths = []
        expected_paths.append(Path(self.job1.working_dir, test_script.name))
        expected_paths.append(Path(self.job1.working_dir, "test_input_sandbox_dir"))
        for i in range(2):
            path = Path(self.job1.working_dir, "test_input_sandbox_dir", f"{i}.txt")
            expected_paths.append(path)

        # Now check that every path in the working directory is one we expect to be there
        for p in self.job1.working_dir.rglob("*"):
            self.assertIn(p, expected_paths)

    def tearDown(self):
        """
        Removes files/directories that were created during these tests
        """
        shutil.rmtree(test_dir)


def main():
    unittest.main()


if __name__ == '__main__':
    main()
