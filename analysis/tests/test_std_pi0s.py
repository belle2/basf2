#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import inspect
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
                if module.type() == 'ParticleLoader' and param.name == 'decayStrings':
                    name = param.values[0].split(':')[1]
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
        """check that the builder function raises a ValueError for a non-existing list name"""
        self.assertRaises(ValueError, self._check_list, "flibble")

    def test_default_list_exists(self):
        """
        Check that the default list type is one of the lists in the cases that are checked for in :func:`stdPi0s.stdPi0s`.

        This test relies on ``ValueError`` being raised for nonsense list types, which is tested by
        :func:`test_nonsense_list`.  However, :func:`test_nonsense_list` doesn't ensure that the default list works, so
        for that this test is needed.
        """
        test_path = create_path()
        try:
            stdPi0s.stdPi0s(path=test_path)
        except ValueError:
            stdPi0s_signature = inspect.signature(stdPi0s.stdPi0s)
            default_listtype = stdPi0s_signature.parameters["listtype"].default
            self.fail(f"stdPi0s default listtype {default_listtype} is not in set of allowed list names.")

    def test_default_list_works(self):
        """Check that the default list type works."""
        stdPi0s_signature = inspect.signature(stdPi0s.stdPi0s)
        default_listtype = stdPi0s_signature.parameters["listtype"].default
        self._check_list(expected_lists=["pi0" + default_listtype, "pi0" + default_listtype, default_listtype])

    def test_all_list(self):
        """check that the builder function works with the all list"""
        self._check_list("all", expected_lists=["all", "all"])

    def test_eff10_May2020_list(self):
        """check that the builder function works with the eff10_May2020 list"""
        self._check_list("eff10_May2020", expected_lists=["pi0eff10_May2020", "pi0eff10_May2020", "eff10_May2020"])

    def test_eff20_May2020_list(self):
        """check that the builder function works with the eff20_May2020 list"""
        self._check_list("eff20_May2020", expected_lists=["pi0eff20_May2020", "pi0eff20_May2020", "eff20_May2020"])

    def test_eff30_May2020_list(self):
        """check that the builder function works with the eff30_May2020 list"""
        self._check_list("eff30_May2020", expected_lists=["pi0eff30_May2020", "pi0eff30_May2020", "eff30_May2020"])

    def test_eff40_May2020_list(self):
        """check that the builder function works with the eff40_May2020 list"""
        self._check_list("eff40_May2020", expected_lists=["pi0eff40_May2020", "pi0eff40_May2020", "eff40_May2020"])

    def test_eff50_May2020_nomcmatch_list(self):
        """check that the builder function works with the eff50_May2020_nomcmatch list"""
        self._check_list("eff50_May2020_nomcmatch",
                         expected_lists=["pi0eff50_May2020",
                                         "pi0eff50_May2020",
                                         "eff50_May2020_nomcmatch"])

    def test_eff50_May2020_list(self):
        """check that the builder function works with the eff50_May2020 list"""
        self._check_list(
            "eff50_May2020",
            expected_lists=[
                "pi0eff50_May2020",
                "pi0eff50_May2020",
                "eff50_May2020_nomcmatch",
                "eff50_May2020"])

    def test_eff60_May2020_list(self):
        """check that the builder function works with the eff60_May2020 list"""
        self._check_list("eff60_May2020", expected_lists=["pi0eff60_May2020", "pi0eff60_May2020", "eff60_May2020"])

    def test_allfit_list(self):
        """check that the builder function works with the allFit list"""
        self._check_list("allFit", expected_lists=["all", "all", "allFit"])

    def test_eff10_May2020fit_list(self):
        """check that the builder function works with the eff10_May2020Fit list"""
        self._check_list(
            "eff10_May2020Fit",
            expected_lists=[
                "pi0eff10_May2020",
                "pi0eff10_May2020",
                "eff10_May2020",
                "eff10_May2020Fit"])

    def test_eff20_May2020fit_list(self):
        """check that the builder function works with the eff20_May2020Fit list"""
        self._check_list(
            "eff20_May2020Fit",
            expected_lists=[
                "pi0eff20_May2020",
                "pi0eff20_May2020",
                "eff20_May2020",
                "eff20_May2020Fit"])

    def test_eff30_May2020fit_list(self):
        """check that the builder function works with the eff30_May2020Fit list"""
        self._check_list(
            "eff30_May2020Fit",
            expected_lists=[
                "pi0eff30_May2020",
                "pi0eff30_May2020",
                "eff30_May2020",
                "eff30_May2020Fit"])

    def test_eff40_May2020fit_list(self):
        """check that the builder function works with the eff40_May2020Fit list"""
        self._check_list(
            "eff40_May2020Fit",
            expected_lists=[
                "pi0eff40_May2020",
                "pi0eff40_May2020",
                "eff40_May2020",
                "eff40_May2020Fit"])

    def test_eff50_May2020fit_list(self):
        """check that the builder function works with the eff50_May2020Fit list"""
        self._check_list(
            "eff50_May2020Fit",
            expected_lists=[
                "pi0eff50_May2020",
                "pi0eff50_May2020",
                'eff50_May2020_nomcmatch',
                "eff50_May2020",
                "eff50_May2020Fit"])

    def test_eff60_May2020fit_list(self):
        """check that the builder function works with the eff60_May2020Fit list"""
        self._check_list(
            "eff60_May2020Fit",
            expected_lists=[
                "pi0eff60_May2020",
                "pi0eff60_May2020",
                "eff60_May2020",
                "eff60_May2020Fit"])

    def test_skim(self):
        """check that the builder function works with the skim list"""
        self._check_list(
            std_function=stdPi0s.loadStdSkimPi0,
            expected_lists=[
                "pi0eff50_May2020",
                "pi0eff50_May2020",
                "eff50_May2020_nomcmatch",
                "skim"])


if __name__ == '__main__':
    unittest.main()
