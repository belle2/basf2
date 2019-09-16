#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
            len(testpath.modules()), len(expected_lists),
            "List %s doesn't work with function %s" % (listtype, std_function.__name__))
        self.assertTrue(all((module.type() == "ParticleLoader") or (module.type() == "ParticleListManipulator")
                            for module in testpath.modules()))

        #
        built_list = []
        for module in testpath.modules():
            for param in module.available_params():
                if param.name == 'decayStringsWithCuts':
                    name = param.values[0][0].split(':')[1]
                    built_list.append(name)
                if param.name == 'outputListName':
                    name = str(param.values).split(':')[1]
                    built_list.append(name)

        # we have the particle lists we expect
        for a, b in zip(built_list, expected_lists):
            self.assertEqual(a, b, "Loaded list \'%s\' instead of \'%s\' with function %s" % (a, b, std_function.__name__))

    def test_nonsense_list(self):
        """check that the builder function works with the all list"""
        self._check_list("flibble", expected_lists=[])

    def test_all_list(self):
        """check that the builder function works with the all list"""
        self._check_list("all", expected_lists=["all"])

    def test_cdc_list(self):
        """check that the builder function works with the cdc list"""
        self._check_list("cdc", expected_lists=["all", "cdc"])

    def test_loose_list(self):
        """check that the builder function works with the loose list"""
        self._check_list("loose", expected_lists=["all", "cdc", "loose"])

    def test_default_list(self):
        """check that the builder function works with the default (loose) list"""
        self._check_list(expected_lists=["all", "cdc", "loose"])

    def test_tight_list(self):
        """check that the builder function works with the tight list"""
        self._check_list("tight", expected_lists=["all", "cdc", "loose", "tight"])

    def test_pi0eff60_list(self):
        """check that the builder function works with the pi0eff60 list"""
        self._check_list("pi0eff60", expected_lists=["all", "pi0eff60"])

    def test_pi0eff50_list(self):
        """check that the builder function works with the pi0eff50 list"""
        self._check_list("pi0eff50", expected_lists=["all", "pi0eff50"])

    def test_pi0eff40_list(self):
        """check that the builder function works with the pi0eff40 list"""
        self._check_list("pi0eff40", expected_lists=["all", "pi0eff40"])

    def test_pi0eff30_list(self):
        """check that the builder function works with the pi0eff30 list"""
        self._check_list("pi0eff30", expected_lists=["all", "pi0eff30"])

    def test_pi0eff20_list(self):
        """check that the builder function works with the pi0eff20 list"""
        self._check_list("pi0eff20", expected_lists=["all", "pi0eff20"])

    def test_pi0_list(self):
        """check that the builder function works with the pi0 list"""
        self._check_list("pi0", expected_lists=["all", "pi0eff60", "pi0"])

    def test_pi0highE_list(self):
        """check that the builder function works with the pi0highE list"""
        self._check_list("pi0highE", expected_lists=["all", "pi0eff60", "pi0highE"])

    def test_skim(self):
        """check that the builder function works with the skim list"""
        self._check_list("skim", std_function=stdPhotons.loadStdSkimPhoton, expected_lists=["all", "cdc", "loose", "skim"])

    def test_belle(self):
        """check that the builder function works with the belle list"""
        self._check_list("goodBelle", std_function=stdPhotons.loadStdGoodBellePhoton, expected_lists=["all", "goodBelle"])


if __name__ == '__main__':
    unittest.main()
