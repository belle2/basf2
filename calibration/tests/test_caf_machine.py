import unittest
from unittest import TestCase

from basf2 import *
import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm as TestAlgo
from caf.framework import Calibration
from caf.state_machines import Machine, CalibrationMachine, MachineError, ConditionError


class Test_Machine(TestCase):
    """
    Set of tests for checking Machine Base class
    """

    def setUp(self):
        """
        """
        #: Machine for testing
        self.m = Machine()
        self.m.add_state("standing")
        self.m.add_state("walking")
        self.m.add_transition("walk", "standing", "walking")
        self.m.add_transition("stop", "walking", "standing")
        self.m.initial_state = "standing"

    def test_initial(self):
        """
        Checks that a new machine sets an initial state correctly
        """
        states = ["walking", "standing"]
        initial = "walking"
        m = Machine(states, initial)
        self.assertEqual(m.state.name, initial)

    def test_set(self):
        """
        Checks that machine sets at a state
        """
        self.m.state = "walking"
        self.assertEqual(self.m.state.name, "walking")

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
        self.assertEqual(self.m.state.name, "walking")

    def test_walk_stop(self):
        """
        Checks that machine can traverse several states
        """
        self.m.state = "standing"
        self.m.walk()
        self.m.stop()
        self.assertEqual(self.m.state.name, "standing")

    def test_true_condition(self):
        """
        Check that when a condition for a transition evaulates true it will transition
        to the correct state
        """
        self.m.add_state("airborne")

        def can_jump():
            return True

        self.m.add_transition("jump", "standing", "airborne", conditions=can_jump)
        self.m.state = "standing"
        self.m.jump()
        self.assertEqual(self.m.state.name, "airborne")

    def test_false_condition(self):
        """
        Check that when a condition for a transition evaulates false it will raise
        the correct error.
        """
        self.m.add_state("airborne")

        def can_jump():
            return False

        self.m.add_transition("jump", "standing", "airborne", conditions=can_jump)
        self.m.state = "standing"
        with self.assertRaises(ConditionError):
            self.m.jump()

    def test_true_conditions(self):
        """
        Check that when multiple conditions for a transition all evaulate true it will transition
        to the correct state
        """
        self.m.add_state("airborne")

        def can_jump():
            return True

        def eaten_lunch():
            return True
        self.m.add_transition("jump", "standing", "airborne", conditions=[can_jump, eaten_lunch])
        self.m.state = "standing"
        self.m.jump()
        self.assertEqual(self.m.state.name, "airborne")

    def test_halftrue_conditions(self):
        """
        Check that when a single condition out of many for a transition evaulates true it will
        fail to move to the correct state
        """
        self.m.add_state("airborne")

        def can_jump():
            return True

        def eaten_lunch():
            return False
        self.m.add_transition("jump", "standing", "airborne", conditions=[can_jump, eaten_lunch])
        self.m.state = "standing"
        with self.assertRaises(ConditionError):
            self.m.jump()


def main():
    unittest.main()


if __name__ == '__main__':
    main()
