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
from variables.utils import create_aliases_for_selected
from variables import variables as vm


class TestVariableUtilities(unittest.TestCase):
    """Test case for the variables.utils functions"""

    #: list of variables to test
    _list_of_variables = ['M', 'p']

    def _expand_expected(self, expected):
        """Expand the list of expected aliases by appending _{name} for each variable in the _list_of_variables"""
        result = []
        for p in expected:
            if p:
                result += [f"{p}_{e}" for e in self._list_of_variables]
            else:
                result += self._list_of_variables
        return result

    def assertAliases(self, decaystring, expected, **argk):
        """Make sure the aliases created for given decaystring are as expected
        Also, pass any additional keyword arguments to the create_aliases_for_selected function
        """
        expected = self._expand_expected(expected)
        actual = create_aliases_for_selected(self._list_of_variables, decaystring, **argk)
        self.assertEqual(expected, actual, f"decaystring: {decaystring}, arguments: {argk}")

    ###########################################################################
    # to add to the tests here, please add a test_something_something for you
    # favourite compicated decay string

    def test_wrong_decaystring(self):
        """Make sure we get an error if the decaystring is not valid"""
        with self.assertRaises(ValueError):
            create_aliases_for_selected(["p"], "eeh <- ok")

    def test_no_particle_selected(self):
        """Make sure we get an error if o particle is selected"""
        with self.assertRaises(ValueError):
            create_aliases_for_selected(["p"], "B0 -> pi0")

    def test_number_of_prefix_mismatch(self):
        """Make sure we get an error if the number of supplied prefixes doesn't
        match the number of selected particles"""
        with self.assertRaises(ValueError):
            create_aliases_for_selected(["p"], "^B0 -> ^pi0", prefix="one")
        with self.assertRaises(ValueError):
            create_aliases_for_selected(["p"], "B0 -> ^pi0", prefix=["one", "two"])

    def test_prefixes_repeated(self):
        """Make sure we got an error if the supplied provided prefixes are not unique"""
        with self.assertRaises(ValueError):
            create_aliases_for_selected(["p"], "^B0 -> ^pi0", prefix=["mine", "mine"])

    def test_named_daughter(self):
        """Check daughter can be selected for an specific named alias"""
        self.assertAliases('B0 -> [^D0 -> pi+ K-] pi0', ['dzero'], prefix='dzero')

    def test_named_mother_and_daughters(self):
        """Check mother and daughter can be selected for an specific named alias"""
        self.assertAliases('^B0 -> [^D0 -> pi+ ^K-] pi0', ['MyB', 'MyD', 'MyK'], prefix=['MyB', 'MyD', 'MyK'])

    def test_unnamed_daughter(self):
        """Check daughter can be selected w/o an specific named alias"""
        self.assertAliases('B0 -> [^D0 -> ^pi+ ^K-] pi0',  ['d0', 'd0_d0', 'd0_d1'], use_names=False)

    def test_unnamed_mother(self):
        """Check daughter can be selected w/o an specific named alias"""
        self.assertAliases('^B0 -> pi0 ^pi0',  ['', 'd1'], use_names=False)
        # also make sure we ignore ``use_relative_indices``
        self.assertAliases('^B0 -> pi0 ^pi0',  ['', 'd1'], use_names=False, use_relative_indices=True)

    def test_autonamed_granddaughter(self):
        """Check granddaughter can be selected for an automatic name alias"""
        self.assertAliases('B0 -> [D0 -> ^pi+ K-] pi0', ['D0_pi'])

    def test_multiple_autoname_granddaughters(self):
        """Check multiple granddaughters can be selected for automatic name aliases"""
        self.assertAliases(
            'B0 -> [D0 -> ^pi+ ^pi- ^pi0] ^pi0',
            [
                'D0_pi_0',
                'D0_pi_1',
                'D0_pi0',
                'pi0',
            ])

    def test_autoindex(self):
        """ check decay-string-of-doom with automatic names """
        self.assertAliases(
            "^B0 -> [D0 -> ^pi+ ^pi-] [D+ -> ^pi+ pi0] [D0 -> ^pi+ ^pi-] [K- -> ^pi+ ^pi-] ^pi+ ^pi- pi0",
            [
                "",
                "D0_0_pi_0",
                "D0_0_pi_1",
                "D_pi",
                "D0_2_pi_0",
                "D0_2_pi_1",
                "K_pi_0",
                "K_pi_1",
                "pi_4",
                "pi_5",
            ])

    def test_autoindex_relative(self):
        """ check decay-string-of-doom with automatic names and relative indexing"""
        self.assertAliases(
            "^B0 -> [D0 -> ^pi+ ^pi-] [D+ -> ^pi+ pi0] [D0 -> ^pi+ ^pi-] [K- -> ^pi+ ^pi-] ^pi+ ^pi- pi0",
            [
                "",
                "D0_0_pi_0",
                "D0_0_pi_1",
                "D_pi",
                "D0_1_pi_0",
                "D0_1_pi_1",
                "K_pi_0",
                "K_pi_1",
                "pi_0",
                "pi_1",
            ], use_relative_indices=True)

    def test_autoindex_allindexed(self):
        """ check decay-string-of-doom with automatic names and forced indices"""
        self.assertAliases(
            "^B0 -> [D0 -> ^pi+ ^pi-] [D+ -> ^pi+ pi0] [D0 -> ^pi+ ^pi-] [K- -> ^pi+ ^pi-] ^pi+ ^pi- pi0",
            [
                "",
                "D0_0_pi_0",
                "D0_0_pi_1",
                "D_1_pi_0",
                "D0_2_pi_0",
                "D0_2_pi_1",
                "K_3_pi_0",
                "K_3_pi_1",
                "pi_4",
                "pi_5",
            ], always_include_indices=True)

    def test_autoindex_allindexed_relative(self):
        """ check decay-string-of-doom with automatic names, relative indexing and forced indices"""
        self.assertAliases(
            "^B0 -> [D0 -> ^pi+ ^pi-] [D+ -> ^pi+ pi0] [D0 -> ^pi+ ^pi-] [K- -> ^pi+ ^pi-] ^pi+ ^pi- pi0",
            [
                "",
                "D0_0_pi_0",
                "D0_0_pi_1",
                "D_0_pi_0",
                "D0_1_pi_0",
                "D0_1_pi_1",
                "K_0_pi_0",
                "K_0_pi_1",
                "pi_0",
                "pi_1",
            ], use_relative_indices=True, always_include_indices=True)

    def test_indexed(self):
        """ check decay-string-of-doom w/o automatic names """
        self.assertAliases(
            "^B0 -> [D0 -> ^pi+ ^pi-] [D+ -> ^pi+ pi0] [D0 -> ^pi+ ^pi-] [K- -> ^pi+ ^pi-] ^pi+ ^pi- pi0",
            [
                "",
                "d0_d0",
                "d0_d1",
                "d1_d0",
                "d2_d0",
                "d2_d1",
                "d3_d0",
                "d3_d1",
                "d4",
                "d5",
            ], use_names=False)

    def test_indexed_relativeignored(self):
        """ check decay-string-of-doom w/o automatic names and make sure relative indexing is **not** honored"""
        self.assertAliases(
            "^B0 -> [D0 -> ^pi+ ^pi-] [D+ -> ^pi+ pi0] [D0 -> ^pi+ ^pi-] [K- -> ^pi+ ^pi-] ^pi+ ^pi- pi0",
            [
                "",
                "d0_d0",
                "d0_d1",
                "d1_d0",
                "d2_d0",
                "d2_d1",
                "d3_d0",
                "d3_d1",
                "d4",
                "d5",
            ], use_names=False, use_relative_indices=True)

    def test_threedkp(self):
        """ check if the indexing works with more than two ... """
        self.assertAliases(
            "B0 -> [D+ -> [K+ -> ^pi+]] [D+ -> [K+ -> ^pi+]] [D+ -> [K+ -> ^pi+]]",
            [
                "D_0_K_pi",
                "D_1_K_pi",
                "D_2_K_pi",
            ])

    def test_fourdkp(self):
        """ check if the indexing works with more than two ... """
        self.assertAliases(
            "B0 -> [D+ -> [K+ -> ^pi+]] [D+ -> [K+ -> ^pi+]] [D+ -> [K+ -> ^pi+]] [D+ -> [K+ -> ^pi+]]",
            [
                "D_0_K_pi",
                "D_1_K_pi",
                "D_2_K_pi",
                "D_3_K_pi",
            ])
        self.assertAliases(
            "B0 -> [D+ -> [K+ -> pi+]] [D+ -> [K+ -> ^pi+]] [D+ -> [^K+ -> pi+]] [D+ -> [K+ -> ^pi+]]",
            [
                "D_1_K_pi",
                "D_2_K",
                "D_3_K_pi",
            ])
        self.assertAliases(
            "B0 -> [D+ -> [K+ -> pi+]] [D+ -> [K+ -> ^pi+]] [D+ -> [^K+ -> pi+]] [D+ -> [K+ -> ^pi+]]",
            [
                "D_0_K_pi",
                "D_1_K",
                "D_2_K_pi",
            ], use_relative_indices=True)

    def test_ohsomany_autonamed(self):
        """Test many many children"""
        self.assertAliases(
            "B0 -> e+:0 ^e+:1 ^e+:2 e+:3 ^e+:4 e+:5 ^e+:6 mu+:7 ^mu+:8 mu+:9 ^mu+:10 ^mu+:11 mu+:12",
            [
                "e_1",
                "e_2",
                "e_4",
                "e_6",
                "mu_8",
                "mu_10",
                "mu_11",
            ])

    def test_ohsomany_autonamed_relative(self):
        """Test many many children with relative indices"""
        self.assertAliases(
            "B0 -> e+:0 ^e+:1 ^e+:2 e+:3 ^e+:4 e+:5 ^e+:6 mu+:7 ^mu+:8 mu+:9 ^mu+:10 ^mu+:11 mu+:12",
            [
                "e_0",
                "e_1",
                "e_2",
                "e_3",
                "mu_0",
                "mu_1",
                "mu_2",
            ], use_relative_indices=True)

    def test_ohsomany_indexed(self):
        """Test many many children without names"""
        for use_relative_indices in True, False:
            for always_include_indices in True, False:
                self.assertAliases(
                    "B0 -> e+:0 ^e+:1 ^e+:2 e+:3 ^e+:4 e+:5 ^e+:6 mu+:7 ^mu+:8 mu+:9 ^mu+:10 ^mu+:11 mu+:12",
                    [
                        "d1",
                        "d2",
                        "d4",
                        "d6",
                        "d8",
                        "d10",
                        "d11",
                    ], use_names=False,
                    use_relative_indices=use_relative_indices,
                    always_include_indices=always_include_indices,
                )

    def test_inclusive(self):
        """Select a decay with the inclusive particle marker"""
        self.assertAliases("B0 -> ^Xsd e+:loose e-:loose", ["Xsd"])

    def test_zfinal(self):
        """Print all aliases as a final check"""
        vm.printAliases()


if __name__ == '__main__':
    unittest.main()
