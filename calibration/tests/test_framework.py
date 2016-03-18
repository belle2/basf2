from basf2 import *

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
        self.cal = Calibration()
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
        self.assertTrue(self.cal.is_valid() and self.cal.name)

    def test_5(self):
        name = 'TestCalibration'
        self.cal.name = name
        self.cal.collector = self.col1
        self.cal.algorithms = [self.alg1, self.alg2]
        self.assertEqual(self.cal.name, name)


class TestCalibrationClass_Args(TestCase):
    def setUp(self):
        self.alg1 = TestAlgo()
        self.alg2 = TestAlgo()
        self.col1 = register_module('CaTest')

    def test_1(self):
        name = 'TestCalibration'
        cal = Calibration(name=name, max_iterations=2, collector=self.col1, algorithms=[self.alg1, self.alg2])
        self.assertTrue(cal.is_valid() and cal.name == name)

    def test_2(self):
        name = 'TestCalibration'
        cal = Calibration('CaTest', self.alg1, name, 2)
        self.assertTrue(cal.is_valid() and cal.name == name)


class TestCalibrationClass_Mismatch(TestCase):
    def setUp(self):
        self.alg1 = TestAlgo()
        self.alg2 = PXDAlgo()
        self.col1 = register_module('CaTest')

    def test_1(self):
        name = 'TestCalibration'
        cal = Calibration(name=name, max_iterations=2, collector=self.col1, algorithms=[self.alg1])
        self.assertTrue(cal.is_valid() and cal.name == name)

    def test_2(self):
        name = 'TestCalibration'
        cal = Calibration(name=name, max_iterations=2, collector=self.col1, algorithms=[self.alg2])
        self.assertFalse(cal.is_valid() and cal.name == name)


class TestCAF(TestCase):
    def setUp(self):
        self.name1 = 'TestCalibration1'
        self.name2 = 'TestCalibration2'
        alg = TestAlgo()
        col = register_module('CaTest')
        self.cal1 = Calibration(col, alg, self.name1)
        self.cal2 = Calibration(col, alg, self.name2)

    def test_add_calibration(self):
        fw = CAF()
        fw.add_calibration(self.cal1)
        fw.add_calibration(self.cal2)
        self.assertTrue(fw.calibrations[0].name == self.name1 and fw.calibrations[1].name == self.name2)

    def test_add_dependency(self):
        fw = CAF()
        fw.add_calibration(self.cal1)
        fw.add_calibration(self.cal2)
        fw.add_dependency(fw.calibrations[0].name, fw.calibrations[1].name)
        self.assertEqual(fw.dependencies[fw.calibrations[0].name][0], fw.calibrations[1].name)


def main():
    unittest.main()

if __name__ == '__main__':
    main()
