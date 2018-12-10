#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from basf2 import create_path
import stdPhotons


class TestStdPhotons(unittest.TestCase):
    """Test case for standard photon lists"""

    def _check_list(self, listtype=None, std_function=stdPhotons.stdPhotons, list_list=["all"]):
        """check that a given listtype function works"""
        testpath = create_path()
        if (std_function is stdPhotons.stdPhotons) and (listtype is not None):
            std_function(listtype, path=testpath)
        else:
            std_function(path=testpath)
        self.assertEqual(
            len(testpath.modules()), len(list_list),
            "List %s doesn't work with function %s" % (listtype, std_function.__name__))
        self.assertTrue(all((module.type() == "ParticleLoader") or (module.type() == "ParticleListManipulator")
                            for module in testpath.modules()))

        # check it was added with the ParticleList name we expect
        tag_list = []
        for module in testpath.modules():
            for param in module.available_params():
                if param.name == 'decayStringsWithCuts':
                    name = param.values[0][0].split(':')[1]
                    tag_list.append(name)
                if param.name == 'outputListName':
                    name = str(param.values).split(':')[1]
                    tag_list.append(name)

        for a, b in zip(tag_list, list_list):
            self.assertEqual(a, b, "Loaded list \'%s\' instead of \'%s\' with function %s" % (a, b, std_function.__name__))

#        print(list(tag_list))
#        result=map(lambda w1, w2: w1==w2, tag_list, list_list)
#        print(list(result))

    def test_nonsense_list(self):
        """check that the builder function works with the all list"""
        self._check_list("flibble", list_list=[])

    def test_all_list(self):
        """check that the builder function works with the all list"""
        self._check_list("all", list_list=["all"])

    def test_cdc_list(self):
        """check that the builder function works with the cdc list"""
        self._check_list("cdc", list_list=["all", "cdc"])

    def test_loose_list(self):
        """check that the builder function works with the loose list"""
        self._check_list("loose", list_list=["all", "cdc", "loose"])

    def test_default_list(self):
        """check that the builder function works with the default (loose) list"""
        self._check_list(list_list=["all", "cdc", "loose"])

    def test_tight_list(self):
        """check that the builder function works with the tight list"""
        self._check_list("tight", list_list=["all", "cdc", "loose", "tight"])

    def test_pi0eff60_list(self):
        """check that the builder function works with the pi0eff60 list"""
        self._check_list("pi0eff60", list_list=["all", "pi0eff60"])

    def test_pi0eff50_list(self):
        """check that the builder function works with the pi0eff50 list"""
        self._check_list("pi0eff50", list_list=["all", "pi0eff50"])

    def test_pi0eff40_list(self):
        """check that the builder function works with the pi0eff40 list"""
        self._check_list("pi0eff40", list_list=["all", "pi0eff40"])

    def test_pi0eff30_list(self):
        """check that the builder function works with the pi0eff30 list"""
        self._check_list("pi0eff30", list_list=["all", "pi0eff30"])

    def test_pi0eff20_list(self):
        """check that the builder function works with the pi0eff20 list"""
        self._check_list("pi0eff20", list_list=["all", "pi0eff20"])

    def test_pi0_list(self):
        """check that the builder function works with the pi0 list"""
        self._check_list("pi0", list_list=["all", "pi0eff60", "pi0"])

    def test_pi0highE_list(self):
        """check that the builder function works with the pi0highE list"""
        self._check_list("pi0highE", list_list=["all", "pi0eff60", "pi0highE"])

    def test_skim(self):
        """check that the builder function works with the skim list"""
        self._check_list("skim", std_function=stdPhotons.loadStdSkimPhoton, list_list=["all", "cdc", "loose", "skim"])

    def test_belle(self):
        """check that the builder function works with the belle list"""
        self._check_list("goodBelle", std_function=stdPhotons.loadStdGoodBellePhoton, list_list=["all", "goodBelle"])


if __name__ == '__main__':
    unittest.main()
