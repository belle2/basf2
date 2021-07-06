#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import unittest
from basf2 import create_path
import stdCharged

#: the basic std functions
_base_functions = [
    stdCharged.stdPi,
    stdCharged.stdK,
    stdCharged.stdPr,
    stdCharged.stdE,
    stdCharged.stdMu
]


class TestStdCharged(unittest.TestCase):
    """Test case for charged standard particle lists"""

    def _check_list_name(self, target, functionname, particleloader):
        """
        Check that the list name we expect is the one that was
        actually added to the ParticleLoader
        """
        for param in particleloader.available_params():
            if param.name == 'decayStrings':
                name = param.values[0].split(':')[1]
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
                len(testpath.modules()), 1 if listtype == 'all' else 3,
                "List %s doesn't work with function %s" % (listtype, f.__name__))
            self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))

            # now we're certain that the loader is the only module on the path,
            # so check it was added with the ParticleList name we expect
            loader = testpath.modules()[0]
            self._check_list_name(listtype, f.__name__, loader)

    def _check_function_call(self, functions=_base_functions, expectedlist=stdCharged._defaultlist):
        """check that a function works (i.e. adds a particle loader)"""
        for f in functions:
            testpath = create_path()
            f(path=testpath)
            self.assertEqual(
                len(testpath.modules()), 3,
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
        """check that the builder functions fail with a nonexisting list"""
        self._check_listtype_does_not_exist("flibble")

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

    def test_loose(self):
        """check that the builder functions all work with the loose list"""
        self._check_listtype_exists("loose")

    def test_percentile_eff(self):
        """check that the builder functions all work with the percentile eff lists"""
        for function in _base_functions:
            eff_exists = 0
            for ename in stdCharged._effnames:
                cut = stdCharged._stdChargedEffCuts(stdCharged._chargednames[_base_functions.index(function)],
                                                    ename)
                if 0.0 < cut < 1.0:
                    self._check_listtype_exists(ename, [function])
                    eff_exists += 1
                else:
                    self._check_listtype_does_not_exist(ename, [function])
            self.assertTrue(
                eff_exists,
                "Function: \"%s\" has no valid list based on efficiency percentile."
                % (function.__name__))

    def test_mostLikely_lists(self):
        """check that the builder functions work with the mostLikely lists"""
        nLists = 5  # Number of ParticleLoader's expected
        testpath = create_path()
        stdCharged.stdMostLikely(path=testpath)
        self.assertEqual(
            len(testpath.modules()), 3 * nLists,
            "There should be %i fillParticleList calls" % nLists)
        self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))
        for module in testpath.modules():
            self._check_list_name('mostlikely', 'stdMostLikely', module)


if __name__ == '__main__':
    unittest.main()
