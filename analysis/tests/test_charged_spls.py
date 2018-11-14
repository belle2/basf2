#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from basf2 import create_path
import spl.stdCharged

#: the basic std functions
_base_functions = {
    spl.stdCharged.stdPi,
    spl.stdCharged.stdK,
    spl.stdCharged.stdPr,
    spl.stdCharged.stdE,
    spl.stdCharged.stdMu
}

#: the functions with 95% lists
_95_functions = {
    spl.stdCharged.stdPi,
    spl.stdCharged.stdK,
    spl.stdCharged.stdMu,
    spl.stdCharged.stdE
}

#: the functions with 90% lists
_90_functions = {
    spl.stdCharged.stdPi,
    spl.stdCharged.stdK,
    spl.stdCharged.stdPr
}

#: the functions with 85% lists
_85_functions = {
    spl.stdCharged.stdPi,
    spl.stdCharged.stdK
}

#: the stdLoose functions
_loose_functions = {
    spl.stdCharged.stdLoosePi,
    spl.stdCharged.stdLooseK,
    spl.stdCharged.stdLoosePr,
    spl.stdCharged.stdLooseE,
    spl.stdCharged.stdLooseMu
}

#: the default particle list loaded
_defaultlist = "good"


class TestStdCharged(unittest.TestCase):
    """Test case for charged standard particle lists"""

    def _check_list_name(self, target, functionname, particleloader):
        """
        Check that the list name we expect is the one that was
        actually added to the ParticleLoader
        """
        for param in particleloader.available_params():
            if param.name == 'decayStringsWithCuts':
                name = param.values[0][0].split(':')[1]
                self.assertTrue(
                    name == target,
                    "Name: \"%s\" added by function %s, expecting \"%s\""
                    % (name, functionname, target))

    def _check_listtype_exists(self, listtype, functions=_base_functions):
        """check that a given listtype function works"""
        for f in functions:
            testpath = create_path()
            f(listtype, path=testpath)
            self.assertEqual(
                len(testpath.modules()), 1,
                "List %s doesn't work with function %s" % (listtype, f.__name__))
            self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))

            # now we're certain that the loader is the only module on the path,
            # so check it was added with the ParticleList name we expect
            loader = testpath.modules()[0]
            self._check_list_name(listtype, f.__name__, loader)

    def _check_function_call(self, functions=_base_functions, expectedlist=_defaultlist):
        """check that a function works (i.e. adds a particle loader)"""
        for f in functions:
            testpath = create_path()
            f(path=testpath)
            self.assertEqual(
                len(testpath.modules()), 1,
                "Function %s doesn't work" % f.__name__)
            self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))
            loader = testpath.modules()[0]
            self._check_list_name(expectedlist, f.__name__, loader)

    def _check_listtype_does_not_exist(self, listtype, functions=_base_functions):
        """check that a given listtype function doesn't do anything"""
        for f in functions:
            testpath = create_path()
            f(listtype, path=testpath)
            self.assertEqual(
                len(testpath.modules()), 0,
                "List %s works with function %s" % (listtype, f.__name__))
            self.assertFalse(any(module.type() == "ParticleLoader" for module in testpath.modules()))

    def test_nonesense_list(self):
        """check that the builder functions all work with the all list"""
        self._check_listtype_does_not_exist("flibble")

    def test_loose(self):
        """check that all functions can be called with no argument"""
        self._check_function_call(_loose_functions, expectedlist="loose")

    def test_all_list(self):
        """check that the builder functions all work with the all list"""
        self._check_listtype_exists("all")

    def test_good_list(self):
        """check that the builder functions all work with the good list"""
        self._check_listtype_exists("good")
        # all functions may be called with no arguments: makes a "good" list
        self._check_function_call(_base_functions)

    def test_higheff(self):
        """check that the builder functions all work with the higheff list"""
        self._check_listtype_exists("higheff")

    def test_95(self):
        """check that the builder functions all work with the 95eff list"""
        self._check_listtype_exists("95eff", _95_functions)
        without_95 = _base_functions - _95_functions
        self._check_listtype_does_not_exist("95eff", without_95)

    def test_90(self):
        """check that the builder functions all work with the 90eff list"""
        self._check_listtype_exists("90eff", _90_functions)
        without_90 = _base_functions - _90_functions
        self._check_listtype_does_not_exist("90eff", without_90)

    def test_85(self):
        """check that the builder functions all work with the 85eff list"""
        self._check_listtype_exists("85eff", _85_functions)
        without_85 = _base_functions - _85_functions
        self._check_listtype_does_not_exist("85eff", without_85)


if __name__ == '__main__':
    unittest.main()
