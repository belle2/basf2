from basf2 import *
import os
import shutil
import glob

# show only Errors as we'll be setting off a lot of ugly deliberate warnings
set_log_level(LogLevel.ERROR)

import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm as TestAlgo
from ROOT.Belle2 import PXDClusterShapeCalibrationAlgorithm as PXDAlgo

from caf.framework import Calibration, CAF

import unittest
from unittest import TestCase


class TestCalibrationClass_Configure(TestCase):
    def setUp(self):
        """
        Create useful objects for each test
        """
        self.cal = Calibration('TestCalibration')
        self.alg1 = TestAlgo()
        self.alg2 = TestAlgo()
        self.col1 = register_module('CaTest')

    def test_1(self):
        self.assertFalse(self.cal.is_valid())

    def test_2(self):
        self.cal.collector = 'CaTest'
        self.assertFalse(self.cal.is_valid())

    def test_3(self):
        self.cal.collector = self.col1
        self.assertFalse(self.cal.is_valid())

    def test_4(self):
        self.cal.collector = self.col1
        self.cal.algorithms = [self.alg1, self.alg2]
        self.assertTrue(self.cal.is_valid())


class TestCalibrationClass_Args(TestCase):
    def setUp(self):
        """
        Create useful objects for each test
        """
        self.alg1 = TestAlgo()
        self.alg2 = TestAlgo()
        self.col1 = register_module('CaTest')
        self.name = 'TestCalibration'

    def test_1(self):
        cal = Calibration(self.name, max_iterations=2, collector=self.col1, algorithms=[self.alg1, self.alg2])
        self.assertTrue(cal.is_valid() and cal.name == self.name)

    def test_2(self):
        cal = Calibration(self.name, 'CaTest', self.alg1, 2)
        self.assertTrue(cal.is_valid() and cal.name == self.name)


class TestCalibrationClass_Mismatch(TestCase):
    def setUp(self):
        """
        Create useful objects for each test
        """
        self.alg1 = TestAlgo()
        self.alg2 = PXDAlgo()
        self.col1 = register_module('CaTest')

    def test_1(self):
        name = 'TestCalibration'
        cal = Calibration(name, max_iterations=2, collector=self.col1, algorithms=[self.alg1])
        self.assertTrue(cal.is_valid() and cal.name == name)

    def test_2(self):
        name = 'TestCalibration'
        cal = Calibration(name, max_iterations=2, collector=self.col1, algorithms=[self.alg2])
        self.assertFalse(cal.is_valid() and cal.name == name)


class TestCAF(TestCase):
    def setUp(self):
        """
        Create useful objects for each test
        """
        self.name1 = 'TestCalibration1'
        self.name2 = 'TestCalibration2'
        self.name3 = 'TestCalibration3'
        alg = TestAlgo()
        col = register_module('CaTest')
        self.cal1 = Calibration(self.name1, col, alg)
        self.cal2 = Calibration(self.name2, col, alg)
        self.cal3 = Calibration(self.name3, col, alg)

    def test_add_calibration(self):
        """
        Test that add_calibration function results in correct output
        """
        fw = CAF()
        fw.add_calibration(self.cal1)
        fw.add_calibration(self.cal2)
        self.assertTrue(fw.calibrations[self.name1].name == self.name1 and fw.calibrations[self.name2].name == self.name2)

    def test_add_dependency(self):
        """
        Test that add_dependency function results in correct output
        """
        fw = CAF()
        fw.add_calibration(self.cal1)
        fw.add_calibration(self.cal2)
        fw.add_dependency(self.name1, self.name2)
        self.assertEqual(fw.dependencies, {self.name1: [], self.name2: [self.name1]})

    def test_order_calibrations(self):
        """
        Test that dependencies can input and order calibrations correctly
        """
        fw = CAF()
        fw.add_calibration(self.cal1)
        fw.add_calibration(self.cal2)
        fw.add_calibration(self.cal3)
        fw.add_dependency(self.cal1.name, self.cal2.name)
        fw.add_dependency(self.cal1.name, self.cal3.name)
        fw.add_dependency(self.cal2.name, self.cal3.name)
        fw.order_calibrations()
        self.assertTrue(fw.order == ['TestCalibration3', 'TestCalibration2', 'TestCalibration1'])

    def test_order_calibrations_cyclic(self):
        """
        Test that cyclic dependencies are correctly identified by the CAF
        """
        fw = CAF()
        fw.add_calibration(self.cal1)
        fw.add_calibration(self.cal2)
        fw.add_calibration(self.cal3)
        fw.add_dependency(self.cal1.name, self.cal2.name)
        fw.add_dependency(self.cal2.name, self.cal3.name)
        fw.add_dependency(self.cal3.name, self.cal1.name)
        self.assertFalse(fw.order_calibrations())

    def test_make_output_dir(self):
        """
        Test that output_path directory is created correctly
        """
        fw = CAF()
        fw.output_path = 'testCAF_outputdir'
        fw._make_output_dir()
        self.assertTrue(os.path.isdir('testCAF_outputdir'))

    def test_make_collector_paths(self):
        """
        Test that collector paths can be serialized into a file
        """
        fw = CAF()
        fw.add_calibration(self.cal1)
        fw.output_path = 'serialise_testCAF_outputdir'
        fw.run()
        self.assertTrue(os.path.isfile(fw.output_path+'/paths/CaTest.pickle'))

    def tearDown(self):
        """
        Removes files that were created during these tests
        """
        dirs = glob.glob('*testCAF_outputdir')
        for directory in dirs:
            shutil.rmtree(directory)


def main():
    unittest.main()

if __name__ == '__main__':
    main()
