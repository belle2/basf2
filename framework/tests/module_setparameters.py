#!/usr/bin/env python3

"""Tests for :func:`basf2.set_module_parameters`"""

import basf2
import unittest


class SetModuleParameters(unittest.TestCase):
    """Test fixture to check setting of parameters for modules in a path by name"""

    def setUp(self):
        """Setup a path with a few EventInfoSetter modules with special names"""
        #: first test module
        self.m1 = basf2.register_module("EventInfoSetter")
        self.m1.set_name("evt1")
        #: second test module
        self.m2 = basf2.register_module("EventInfoSetter")
        self.m2.set_name("evt2")
        #: third test module
        self.m3 = basf2.register_module("EventInfoSetter")
        self.m3.set_name("evt1")
        #: main test path
        self.path = basf2.create_path()
        #: secondary test path
        self.subpath = basf2.create_path()
        self.path.add_module(self.m1)
        self.path.add_module(self.m2)
        self.subpath.add_module(self.m3)

    def check_parameters(self, module, **params):
        """Check if the parameters of a module are set explicitly to a given value

        module: basf2.Module instance
        params: named parameters of to check for
        """
        for p in module.available_params():
            if p.name in params:
                self.assertTrue(p.setInSteering)
                self.assertEqual(p.values, params[p.name])

    def check_unset(self, module):
        """Check that all parameters of a given module are not set explicitely"""
        for p in module.available_params():
            self.assertFalse(p.setInSteering)

    def test_simple(self):
        """Check setting the parameters for one module and that it does not affect the other modules"""
        self.check_unset(self.m1)
        basf2.set_module_parameters(self.path, "evt1", evtNumList=[1, 2, 3])
        self.check_parameters(self.m1, evtNumList=[1, 2, 3])
        self.check_unset(self.m2)
        self.check_unset(self.m3)
        basf2.set_module_parameters(self.path, "evt2", evtNumList=[4])
        self.check_parameters(self.m1, evtNumList=[1, 2, 3])
        self.check_parameters(self.m2, evtNumList=[4])
        self.check_unset(self.m3)

    def test_double(self):
        """Check that it also works if more than one module with the name exists"""
        self.path.add_module(self.m3)
        basf2.set_module_parameters(self.path, "evt1", evtNumList=[1, 2, 3])
        self.check_parameters(self.m1, evtNumList=[1, 2, 3])
        self.check_unset(self.m2)
        self.check_parameters(self.m3, evtNumList=[1, 2, 3])

    def test_empty(self):
        """Make sure that not supplying any parameters gives a ValueError"""
        with self.assertRaises(ValueError):
            basf2.set_module_parameters(self.path, "evt1")

    def test_missing(self):
        """Make sure that not finding any module of the given name raises a KeyError"""
        with self.assertRaises(KeyError):
            basf2.set_module_parameters(self.path, "evt3", evtNumList=[0])

    def test_undefined_param(self):
        """Make sure that being unable to set the parameter raises a RuntimeError"""
        with self.assertRaises(RuntimeError):
            basf2.set_module_parameters(self.path, "evt1", foo="bar")

    def test_condition(self):
        """check that modules in a condition sub path are not affected by default"""
        self.m2.if_true(self.subpath)
        basf2.set_module_parameters(self.path, "evt1", evtNumList=[1, 2, 3])
        self.check_parameters(self.m1, evtNumList=[1, 2, 3])
        self.check_unset(self.m2)
        self.check_unset(self.m3)

    def test_condition_recursive(self):
        """check that modules in a condition sub path are affected in recursive mode"""
        self.m2.if_true(self.subpath)
        basf2.set_module_parameters(self.path, "evt1", evtNumList=[1, 2, 3], recursive=True)
        self.check_parameters(self.m1, evtNumList=[1, 2, 3])
        self.check_unset(self.m2)
        self.check_parameters(self.m3, evtNumList=[1, 2, 3])

    def test_subevent(self):
        """check that modules in a for_each sub path are not affected by default"""
        self.path.for_each("foo", "foos", self.subpath)
        basf2.set_module_parameters(self.path, "evt1", evtNumList=[1, 2, 3])
        self.check_parameters(self.m1, evtNumList=[1, 2, 3])
        self.check_unset(self.m2)
        self.check_unset(self.m3)

    def test_subevent_recursive(self):
        """check that modules in a for_each sub path are affected in recursive mode"""
        self.path.for_each("foo", "foos", self.subpath)
        basf2.set_module_parameters(self.path, "evt1", evtNumList=[1, 2, 3], recursive=True)
        self.check_parameters(self.m1, evtNumList=[1, 2, 3])
        self.check_unset(self.m2)
        self.check_parameters(self.m3, evtNumList=[1, 2, 3])

    def test_condition_subevent(self):
        """check that it also works with a conditions and a for_each sub path in recursive mode"""
        sub2 = basf2.create_path()
        m4 = sub2.add_module("EventInfoSetter")
        m4.set_name("evt1")
        self.m2.if_true(self.subpath)
        self.path.for_each("foo", "foos", sub2)
        basf2.set_module_parameters(self.path, "evt1", evtNumList=[1, 2, 3], recursive=True)
        self.check_parameters(self.m1, evtNumList=[1, 2, 3])
        self.check_unset(self.m2)
        self.check_parameters(self.m3, evtNumList=[1, 2, 3])
        self.check_parameters(m4, evtNumList=[1, 2, 3])


if __name__ == '__main__':
    unittest.main()
