import unittest
from unittest import TestCase

from basf2 import *
import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm as TestAlgo
from caf.framework import Calibration
from caf.state_machines import Machine, CalibrationMachine, MachineError


class Test_Machine(TestCase):
    """
    Set of tests for checking Machine Base class
    """
    def setUp(self):
        """
        """
        self.m = Machine()
        self.m.add_state("standing")
        self.m.add_state("walking")
        self.m.add_transition("walk", "standing", "walking")
        self.m.add_transition("stop", "walking", "standing")

    def test_set(self):
        """
        Checks that machine sets at a state
        """
        self.m.state = "walking"
        self.assertTrue(self.m.state == "walking")

    def test_bad_set(self):
        """
        Checks that machine won't set a state not in the list
        """
        with self.assertRaises(MachineError):
            self.m.state = "fake"

    def test_walk(self):
        """
        Checks that machine can transition to a state
        """
        self.m.state = "standing"
        self.m.walk()
        self.assertTrue(self.m.state == "walking")

    def test_walk_stop(self):
        """
        Checks that machine can traverse several states
        """
        self.m.state = "standing"
        self.m.walk()
        self.m.stop()
        self.assertTrue(self.m.state == "standing")


class Test_CalibrationMachine(TestCase):
    def setUp(self):
        """
        """
        col = register_module('CaTest')
        alg = TestAlgo()
        cal = Calibration('TestCalibrationClass_Configure_test5')
        cal.collector = col
        cal.algorithms = alg
        cal.input_files = '/path/to/file.root'
        self.cal = cal

    def test_init(self):
        """
        Checks that the default init state was setup
        """
        cm = CalibrationMachine(self.cal)
        self.assertTrue(cm.state == "init")

    def test_traverse_full(self):
        """
        Tests the correct path of states from beginning to end
        """
        cm = CalibrationMachine(self.cal)
        cm.submit_collector()
        cm.complete()
        cm.run_algorithm()
        cm.complete()
        cm.finish()
        self.assertTrue(cm.state == "completed")


def main():
    unittest.main()

if __name__ == '__main__':
    main()
