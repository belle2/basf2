#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from basf2 import create_path
import stdPi0s


class TestStdPi0s(unittest.TestCase):
    """Test case for standard pi0 lists"""

    def _check_list(self, listtype=None, std_function=stdPi0s.stdPi0s, expected_lists=["all"]):
        """check that a given listtype function works"""
        testpath = create_path()
        if (std_function is stdPi0s.stdPi0s) and (listtype is not None):
            std_function(listtype, path=testpath)
        else:
            std_function(path=testpath)

        built_lists = []
        for module in testpath.modules():
            for param in module.available_params():
                if module.type() == 'ParticleLoader' and param.name == 'decayStringsWithCuts':
                    name = param.values[0][0].split(':')[1]
                    built_lists.append(name)
                if module.type() == 'ParticleListManipulator' and param.name == 'outputListName':
                    name = str(param.values).split(':')[1]
                    built_lists.append(name)
                if module.type() == 'ParticleCombiner' and param.name == 'decayString':
                    name = param.values.split(':')[1].split(' -> ')[0]
                    built_lists.append(name)

        # we have the particle lists we expect
        for a, b in zip(built_lists, expected_lists):
            self.assertEqual(a, b, "Loaded list \'%s\' instead of \'%s\' with function %s" % (a, b, std_function.__name__))

        print(list(built_lists))
        result = map(lambda w1, w2: w1 == w2, built_lists, expected_lists)
        print(list(result))

    def test_nonsense_list(self):
        """check that the builder function works with the all list"""
        self._check_list("flibble", expected_lists=[])

    def test_all_list(self):
        """check that the builder function works with the all list"""
        self._check_list("all", expected_lists=["all", "all"])

    def test_default_list(self):
        """check that the builder function works with the default (veryLoose) list"""
        self._check_list(expected_lists=["all", "pi0eff60", "eff60_nomcmatch", "eff60", "veryLoose"])

    def test_eff20_list(self):
        """check that the builder function works with the eff20 list"""
        self._check_list("eff20", expected_lists=["all", "pi0eff20", "eff20"])

    def test_eff30_list(self):
        """check that the builder function works with the eff30 list"""
        self._check_list("eff30", expected_lists=["all", "pi0eff30", "eff30"])

    def test_eff40_list(self):
        """check that the builder function works with the eff40 list"""
        self._check_list("eff40", expected_lists=["all", "pi0eff40", "eff40"])

    def test_eff50_list(self):
        """check that the builder function works with the eff50 list"""
        self._check_list("eff50", expected_lists=["all", "pi0eff50", "eff50"])

    def test_eff60_nomcmatch_list(self):
        """check that the builder function works with the eff60_nomcmatch list"""
        self._check_list("eff60_nomcmatch", expected_lists=["all", "pi0eff60", "eff60_nomcmatch"])

    def test_eff60_list(self):
        """check that the builder function works with the eff60 list"""
        self._check_list("eff60", expected_lists=["all", "pi0eff60", "eff60_nomcmatch", "eff60"])

    def test_veryloose_list(self):
        """check that the builder function works with the veryLoose list"""
        self._check_list("veryLoose", expected_lists=["all", "pi0eff60", "eff60_nomcmatch", "eff60", "veryLoose"])

    def test_loose_list(self):
        """check that the builder function works with the loose list"""
        self._check_list("loose", expected_lists=["all", "pi0eff40", "eff40", "loose"])

    def test_veryloosefit_list(self):
        """check that the builder function works with the veryLooseFit list"""
        self._check_list(
            "veryLooseFit",
            expected_lists=[
                "all",
                "pi0eff60",
                "eff60_nomcmatch",
                "eff60",
                "veryLoose",
                "veryLooseFit"])

    def test_loosefit_list(self):
        """check that the builder function works with the looseFit list"""
        self._check_list("looseFit", expected_lists=["all", "pi0eff40", "eff40", "loose", "looseFit"])

    def test_allfit_list(self):
        """check that the builder function works with the allFit list"""
        self._check_list("allFit", expected_lists=["all", "all", "allFit"])

    def test_eff20fit_list(self):
        """check that the builder function works with the eff20Fit list"""
        self._check_list("eff20Fit", expected_lists=["all", "pi0eff20", "eff20", "eff20Fit"])

    def test_eff30fit_list(self):
        """check that the builder function works with the eff30Fit list"""
        self._check_list("eff30Fit", expected_lists=["all", "pi0eff30", "eff30", "eff30Fit"])

    def test_eff40fit_list(self):
        """check that the builder function works with the eff40Fit list"""
        self._check_list("eff40Fit", expected_lists=["all", "pi0eff40", "eff40", "eff40Fit"])

    def test_eff50fit_list(self):
        """check that the builder function works with the eff50Fit list"""
        self._check_list("eff50Fit", expected_lists=["all", "pi0eff50", "eff50", "eff50Fit"])

    def test_eff60fit_list(self):
        """check that the builder function works with the eff60Fit list"""
        self._check_list("eff60Fit", expected_lists=["all", "pi0eff60", "eff60_nomcmatch", "eff60", "eff60Fit"])

    def test_skim(self):
        """check that the builder function works with the skim list"""
        self._check_list(std_function=stdPi0s.loadStdSkimPi0, expected_lists=["all", "pi0eff60", "eff60_nomcmatch", "skim"])


if __name__ == '__main__':
    unittest.main()
