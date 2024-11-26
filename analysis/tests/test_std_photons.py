#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import inspect
import unittest

from basf2 import create_path
import stdPhotons


class TestStdPhotons(unittest.TestCase):
    """Test case for standard photon lists"""

    def _check_list(self, listtype=None, std_function=stdPhotons.stdPhotons, expected_lists=["all"]):
        """check that a given listtype function works"""
        testpath = create_path()
        if (std_function is stdPhotons.stdPhotons) and (listtype is not None):
            std_function(listtype, path=testpath)
        else:
            std_function(path=testpath)

        # verify that we load only the list-creating modules
        self.assertEqual(
            len(testpath.modules()), len(set(expected_lists)) + 1 if listtype == 'all' else len(set(expected_lists)) + 3,
            f"List {listtype} doesn't work with function {std_function.__name__}")
        self.assertTrue(all((module.type() == "ParticleLoader") or (module.type() == "ParticleListManipulator")
                            or (module.type() == "ParticleSelector")
                            for module in testpath.modules()))

        #
        built_list = []
        for module in testpath.modules():
            for param in module.available_params():
                if param.name == 'decayStrings':
                    name = param.values[0].split(':')[1]
                    built_list.append(name)
                if param.name == 'outputListName':
                    name = str(param.values).split(':')[1]
                    built_list.append(name)

        # we have the particle lists we expect
        for a, b in zip(built_list, expected_lists):
            self.assertEqual(a, b, f"Loaded list \'{a}\' instead of \'{b}\' with function {std_function.__name__}")

    def test_nonsense_list(self):
        """Check that the builder function raises a ValueError for a non-existing list name."""
        self.assertRaises(ValueError, self._check_list, "flibble")

    def test_default_list_exists(self):
        """
        Check that the default list type is one of the lists in the cases that are checked for in :func:`stdPhotons.stdPhotons`.

        This test relies on ``ValueError`` being raised for nonsense list types, which is tested by
        :func:`test_nonsense_list`.  However, :func:`test_nonsense_list` doesn't ensure that the default list works, so
        for that this test is needed.
        """
        test_path = create_path()
        try:
            stdPhotons.stdPhotons(path=test_path)
        except ValueError:
            stdPhotons_signature = inspect.signature(stdPhotons.stdPhotons)
            default_listtype = stdPhotons_signature.parameters["listtype"].default
            self.fail(f"stdPhotons default listtype {default_listtype} is not in set of allowed list names.")

    def test_default_list_works(self):
        """Check that the default list type (loose) works."""
        # basically a duplicate of test_loose_list
        self._check_list(expected_lists=["cdc", "cdc", "loose"])

    def test_all_list(self):
        """check that the builder function works with the all list"""
        self._check_list("all", expected_lists=["all"])

    def test_cdc_list(self):
        """check that the builder function works with the cdc list"""
        self._check_list("cdc", expected_lists=["cdc"])

    def test_loose_list(self):
        """check that the builder function works with the loose list"""
        self._check_list("loose", expected_lists=["cdc", "cdc", "loose"])

    def test_default_list(self):
        """check that the builder function works with the default (loose) list"""
        self._check_list(expected_lists=["cdc", "cdc", "loose"])

    def test_tight_list(self):
        """check that the builder function works with the tight list"""
        self._check_list("tight", expected_lists=["cdc", "cdc", "loose", "tight"])

    def test_pi0eff60_May2020_list(self):
        """check that the builder function works with the pi0eff60_May2020 list"""
        self._check_list("pi0eff60_May2020", expected_lists=["pi0eff60_May2020"])

    def test_pi0eff50_May2020_list(self):
        """check that the builder function works with the pi0eff50_May2020 list"""
        self._check_list("pi0eff50_May2020", expected_lists=["pi0eff50_May2020"])

    def test_pi0eff40_May2020_list(self):
        """check that the builder function works with the pi0eff40_May2020 list"""
        self._check_list("pi0eff40_May2020", expected_lists=["pi0eff40_May2020"])

    def test_pi0eff30_May2020_list(self):
        """check that the builder function works with the pi0eff30_May2020 list"""
        self._check_list("pi0eff30_May2020", expected_lists=["pi0eff30_May2020"])

    def test_pi0eff20_May2020_list(self):
        """check that the builder function works with the pi0eff20_May2020 list"""
        self._check_list("pi0eff20_May2020", expected_lists=["pi0eff20_May2020"])

    def test_pi0eff10_May2020_list(self):
        """check that the builder function works with the pi0eff10_May2020 list"""
        self._check_list("pi0eff10_May2020", expected_lists=["pi0eff10_May2020"])

    def test_skim(self):
        """check that the builder function works with the skim list"""
        self._check_list("skim", std_function=stdPhotons.loadStdSkimPhoton, expected_lists=["cdc", "cdc", "loose", "skim"])

    def test_belle(self):
        """check that the builder function works with the belle list"""
        self._check_list("goodBelle", std_function=stdPhotons.loadStdGoodBellePhoton, expected_lists=["goodBelle"])


if __name__ == '__main__':
    unittest.main()
