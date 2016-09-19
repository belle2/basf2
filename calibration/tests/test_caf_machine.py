import unittest
from unittest import TestCase

from caf.state_machines import Machine


class A():
    def __init__(self, name):
        self.name = name

    def f(self):
        print(name)


class Test_Machine(TestCase):
    """
    Set of tests for checking Machine Base class
    """

    def test_walk_stop(self):
        """
        Checks that int can be decoded from config file
        """
        m = Machine()
        m.add_state("standing")
        m.add_state("walking")
        m.state = "standing"
        m.add_transition("walk", "standing", "walking")
        m.add_transition("stop", "walking", "standing")
        m.walk()
        m.stop()
        self.assertTrue(m.state == "standing")


def main():
    unittest.main()

if __name__ == '__main__':
    main()
