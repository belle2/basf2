#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from basf2 import create_path
import stdPhotons

_default_function = stdPhotons.stdPhotons
_skim_function = stdPhotons.loadStdSkimPhoton
_belle_function = stdPhotons.loadStdGoodBellePhoton

_defaultlist = "loose"


class TestStdPhotons(unittest.TestCase):
    """Test case for standard photon lists"""

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

    def _check_listtype_exists(self, listtype, f=_default_function, expected_modules_loaded=1):
        """check that a given listtype function works"""
        testpath = create_path()
        f(listtype, path=testpath)
        self.assertEqual(
            len(testpath.modules()), expected_modules_loaded,
            "List %s doesn't work with function %s" % (listtype, f.__name__))
        self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))

        # now we're certain that the loader is the only module on the path,
        # so check it was added with the ParticleList name we expect
        loader = testpath.modules()[expected_modules_loaded - 1]
#        self._check_list_name(listtype, f.__name__, loader)

    def _check_function_call(self, f=_default_function, expectedlist=_defaultlist, expected_modules_loaded=1):
        """check that a function works (i.e. adds a particle loader)"""
        testpath = create_path()
        f(path=testpath)
        self.assertEqual(
            len(testpath.modules()), expected_modules_loaded,
            "Function %s doesn't work" % f.__name__)
        self.assertTrue(any(module.type() == "ParticleLoader" for module in testpath.modules()))
        loader = testpath.modules()[expected_modules_loaded - 1]
#        self._check_list_name(expectedlist, f.__name__, loader)

    def _check_listtype_does_not_exist(self, listtype, f=_default_function):
        """check that a given listtype function doesn't do anything"""
        testpath = create_path()
        f(listtype, path=testpath)
        self.assertEqual(
            len(testpath.modules()), 0,
            "List %s works with function %s" % (listtype, f.__name__))
        self.assertFalse(any(module.type() == "ParticleLoader" for module in testpath.modules()))

    def test_nonesense_list(self):
        """check that the builder function works with the all list"""
        self._check_listtype_does_not_exist("flibble")

    def test_all_list(self):
        """check that the builder function works with the all list"""
        self._check_listtype_exists("all", expected_modules_loaded=1)  # all

    def test_cdc_list(self):
        """check that the builder function works with the cdc list"""
        self._check_listtype_exists("cdc", expected_modules_loaded=2)  # all,cdc

    def test_loose_list(self):
        """check that the builder function works with the loose list"""
        self._check_listtype_exists("loose", expected_modules_loaded=3)  # all,cdc,loose
        # all functions may be called with no arguments: makes a "good" list
        self._check_function_call(_default_function, expected_modules_loaded=3)  # all,cdc,loose

    def test_tight_list(self):
        """check that the builder function works with the tight list"""
        self._check_listtype_exists("tight", expected_modules_loaded=4)  # all,cdc,loose,tight

    def test_pi0eff60_list(self):
        """check that the builder function works with the pi0eff60 list"""
        self._check_listtype_exists("pi0eff60", expected_modules_loaded=2)  # all,pi0eff60

    def test_pi0eff50_list(self):
        """check that the builder function works with the pi0eff50 list"""
        self._check_listtype_exists("pi0eff50", expected_modules_loaded=2)  # all,pi0eff50

    def test_pi0eff40_list(self):
        """check that the builder function works with the pi0eff40 list"""
        self._check_listtype_exists("pi0eff40", expected_modules_loaded=2)  # all,pi0eff40

    def test_pi0eff30_list(self):
        """check that the builder function works with the pi0eff30 list"""
        self._check_listtype_exists("pi0eff30", expected_modules_loaded=2)  # all,pi0eff30

    def test_pi0eff20_list(self):
        """check that the builder function works with the pi0eff20 list"""
        self._check_listtype_exists("pi0eff20", expected_modules_loaded=2)  # all,pi0eff20

    def test_pi0_list(self):
        """check that the builder function works with the pi0 list"""
        self._check_listtype_exists("pi0", expected_modules_loaded=3)  # all,pi0eff60,pi0

    def test_pi0highE_list(self):
        """check that the builder function works with the pi0highE list"""
        self._check_listtype_exists("pi0highE", expected_modules_loaded=3)  # all,pi0eff60,pi0highE

    def test_skim(self):
        """check that the builder function works with the skim list"""
        self._check_function_call(_skim_function, expectedlist="skim", expected_modules_loaded=4)  # all,cdc,loose,skim

    def test_belle(self):
        """check that the builder function works with the belle list"""
        self._check_function_call(_belle_function, expectedlist="goodBelle", expected_modules_loaded=2)  # all,goodBelle


if __name__ == '__main__':
    unittest.main()
