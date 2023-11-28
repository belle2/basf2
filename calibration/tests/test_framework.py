# disable doxygen check for this file
# @cond

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2
import os
import shutil
import glob

from ROOT.Belle2 import TestCalibrationAlgorithm as TestAlgo

from caf.framework import Calibration, CAF

import unittest
from unittest import TestCase
from pathlib import Path

# show only Errors as we'll be setting off a lot of ugly deliberate warnings
b2.set_log_level(b2.LogLevel.ERROR)


class TestCalibrationClass_Configure(TestCase):
    """
    UnitTest for configuration of Calibration class
    """

    def setUp(self):
        """
        Create useful objects for each test
        """
        #: Calibration algorithm for use in unittests
        self.alg1 = TestAlgo()
        #: Calibration algorithm attribute for use in unittests
        self.alg2 = TestAlgo()
        #: Collector module attribute for use in unittests
        self.col1 = b2.register_module('CaTest')
        #: Path to an example file
        self.example_file1 = Path("example1.root")
        self.example_file2 = Path("example2.root")
        self.example_file1.touch()
        self.example_file2.touch()

    def test_1(self):
        """
        Test whether or not calibration is valid with incorrect setup.
        """
        cal = Calibration('TestCalibrationClass_Configure_test1')
        self.assertFalse(cal.is_valid())

    def test_2(self):
        """
        Test whether or not calibration is valid with incorrect setup.
        """
        cal = Calibration('TestCalibrationClass_Configure_test2')
        cal.collector = 'CaTest'
        self.assertFalse(cal.is_valid())

    def test_3(self):
        """
        Test whether or not calibration is valid with incorrect setup.
        """
        cal = Calibration('TestCalibrationClass_Configure_test3')
        cal.collector = self.col1
        self.assertFalse(cal.is_valid())

    def test_4(self):
        """
        Test whether or not calibration is valid with incorrect setup.
        """
        cal = Calibration('TestCalibrationClass_Configure_test4')
        cal.collector = self.col1
        cal.algorithms = [self.alg1, self.alg2]
        self.assertFalse(cal.is_valid())

    def test_5(self):
        """
        Test whether or not calibration is valid with correct setup.
        """
        cal = Calibration('TestCalibrationClass_Configure_test5')
        cal.collector = self.col1
        cal.algorithms = [self.alg1, self.alg2]
        cal.input_files = self.example_file1.as_posix()
        self.assertTrue(cal.is_valid())

    def test_6(self):
        """
        Test whether or not calibration is valid with alternative correct setup.
        """
        cal = Calibration('TestCalibrationClass_Configure_test6')
        cal.collector = self.col1
        cal.algorithms = [self.alg1, self.alg2]
        cal.input_files = [self.example_file1.as_posix(), self.example_file2.as_posix()]
        self.assertTrue(cal.is_valid())

    def tearDown(self):
        self.example_file1.unlink()
        self.example_file2.unlink()


class TestCalibrationClass_Args(TestCase):
    """
    UnitTest for validity of Calibration class when given arguments of different types
    """

    def setUp(self):
        """
        Create useful objects for each test
        """
        #: Calibration algorithm for use in unittests
        self.alg1 = TestAlgo()
        #: Calibration algorithm for use in unittests
        self.alg2 = TestAlgo()
        #: Collector module attribute for use in unittests
        self.col1 = b2.register_module('CaTest')
        #: Calibration name for use in unittests
        self.name = 'TestCalibration'
        #: Path to an example file
        self.example_file1 = Path("example1.root")
        self.example_file2 = Path("example2.root")
        self.example_file1.touch()
        self.example_file2.touch()

    def test_1(self):
        """
        Test whether or not calibration is valid with correct setup and if name is stored correctly
        """
        cal = Calibration(
            self.name,
            collector=self.col1,
            algorithms=[
                self.alg1,
                self.alg2],
            input_files=self.example_file2.as_posix())
        self.assertTrue(cal.is_valid() and cal.name == self.name)

    def test_2(self):
        """
        Test whether or not calibration is valid with alternative correct setup and if name is stored correctly
        """
        cal = Calibration(self.name, 'CaTest', self.alg1, input_files=self.example_file1.as_posix())
        self.assertTrue(cal.is_valid() and cal.name == self.name)

    def test_3(self):
        """
        Test that the default options are correctly applied to a calibration
        """
        defaults = {"max_iterations": 4}
        cal1 = Calibration(self.name, collector=self.col1, algorithms=[self.alg1], input_files=self.example_file1.as_posix())
        cal1._apply_calibration_defaults(defaults)
        cal2 = Calibration(self.name, collector=self.col1, algorithms=[self.alg1], input_files=self.example_file2.as_posix())
        self.assertTrue(cal1.max_iterations == 4 and not cal2.max_iterations)

    def tearDown(self):
        self.example_file1.unlink()
        self.example_file2.unlink()


class TestCAF(TestCase):
    """
    UnitTest for configuration and simple running of CAF
    """

    def setUp(self):
        """
        Create useful objects for each test and the teardown
        """
        #: Calibration name for use in unittests
        self.name1 = 'TestCalibration1'
        #: Calibration name for use in unittests
        self.name2 = 'TestCalibration2'
        #: Calibration name for use in unittests
        self.name3 = 'TestCalibration3'
        alg = TestAlgo()
        col = b2.register_module('CaTest')
        #: Path to an example file
        self.example_file1 = Path("example1.root")
        self.example_file1.touch()
        #: Calibration attribute for use in unittests
        self.cal1 = Calibration(self.name1, col, alg, self.example_file1.as_posix())
        #: Calibration attribute for use in unittests
        self.cal2 = Calibration(self.name2, col, alg, self.example_file1.as_posix())
        #: Calibration attribute for use in unittests
        self.cal3 = Calibration(self.name3, col, alg, self.example_file1.as_posix())

    def test_add_calibration(self):
        """
        Test that add_calibration function results in correct output
        """
        fw = CAF()
        fw.add_calibration(self.cal1)
        fw.add_calibration(self.cal2)
        self.assertTrue(fw.calibrations[self.name1].name == self.name1 and fw.calibrations[self.name2].name == self.name2)

    def test_add_dependency_on_itself(self):
        """
        Test that add_dependency function cannot add itself
        """
        self.cal1.depends_on(self.cal1)
        self.assertFalse(self.cal1.dependencies)

    def test_make_output_dir(self):
        """
        Test that output_dir directory is created correctly
        """
        fw = CAF()
        fw.output_dir = 'testCAF_outputdir'
        fw._make_output_dir()
        self.assertTrue(os.path.isdir('testCAF_outputdir'))

    def test_config_output_dir(self):
        """
        Test that config is correctly setting the default output path.
        """
        fw = CAF()
        self.assertTrue(fw.output_dir == 'calibration_results')

    def tearDown(self):
        """
        Removes files that were created during these tests
        """
        self.example_file1.unlink()
        dirs = glob.glob('*testCAF_outputdir')
        for directory in dirs:
            shutil.rmtree(directory)


def main():
    unittest.main()


if __name__ == '__main__':
    main()

# @endcond
