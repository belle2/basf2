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
import root_pandas
import basf2 as b2
import b2test_utils as b2tu
import modularAnalysis as ma
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from variables import variables as vm


def dump_3_v2nts(names_of_three_lists, path):
    """Dump each of the three vpho lists into a VariablesToNtuple on path"""
    first, second, third = names_of_three_lists
    vs = ['d0_mdstIndex', 'd0_p']
    ma.variablesToNtuple("vpho:" + first,  vs, treename=first,  path=path)
    ma.variablesToNtuple("vpho:" + second, vs, treename=second, path=path)
    ma.variablesToNtuple("vpho:" + third,  vs, treename=third,  path=path)


def run_copylists():
    """A simple basf2 analysis job to do some list copying"""

    # aliases for useful for manual debugging
    vm.addAlias("d0_mdstIndex", "daughter(0, mdstIndex)")
    vm.addAlias("d0_p", "daughter(0, p)")

    # analysis input
    pa = b2.Path()
    ma.inputMdst("default", b2.find_file("analysis/tests/mdst.root"), path=pa)
    ma.fillParticleList("pi+", "", path=pa)
    ma.fillParticleList("K+", "", path=pa)
    stdPi0s('all', path=pa)
    stdKshorts(path=pa)

    # first test: check that merging two lists with
    # identical daughters removes duplicates
    ma.reconstructDecay("vpho:a -> K+", "", path=pa, allowChargeViolation=True)
    ma.reconstructDecay("vpho:b -> K+", "", path=pa, allowChargeViolation=True)
    ma.copyLists("vpho:ab", ["vpho:a", "vpho:b"], path=pa)
    dump_3_v2nts(["a", "b", "ab"], path=pa)

    # second test: check that two lists with different daughters (but from the
    # same underlying mdst object) are included twice (as expected)
    ma.reconstructDecay("vpho:c -> K+", "", path=pa, allowChargeViolation=True)
    ma.reconstructDecay("vpho:d -> pi-", "", path=pa, allowChargeViolation=True)
    ma.copyLists("vpho:cd", ["vpho:c", "vpho:d"], path=pa)
    dump_3_v2nts(["c", "d", "cd"], path=pa)

    # third test: check that two lists with the same daughers in different
    # orders doesn't double count
    # (they are different Belle2::Particles but we should match them)
    ma.reconstructDecay("vpho:e -> K+ pi-", "", path=pa)
    ma.reconstructDecay("vpho:f -> pi- K+", "", path=pa)
    ma.copyLists("vpho:ef", ["vpho:e", "vpho:f"], path=pa)
    dump_3_v2nts(["e", "f", "ef"], path=pa)

    # fourth test: check that it is considered whose daughter a pi0 is when
    # there are two charge-conjugated daughters
    ma.reconstructDecay("tau+:pi -> pi+", "", dmID=0, path=pa)
    ma.reconstructDecay("tau+:pipi0 -> pi+ pi0:all", "", dmID=1, path=pa)
    # the pi0 is the daughter of the tau-
    ma.reconstructDecay("vpho:g -> tau+:pi tau-:pipi0", "", path=pa, chargeConjugation=False)
    # the pi0 is the daughter of the tau+
    ma.reconstructDecay("vpho:h -> tau-:pi tau+:pipi0", "", path=pa, chargeConjugation=False)
    # charge-conjugation has been turned off for the vpho lists above so the
    # merged list should just be the sum
    ma.copyLists("vpho:gh", ["vpho:g", "vpho:h"], path=pa)
    dump_3_v2nts(["g", "h", "gh"], path=pa)

    # fifth test: check that it is considered whose daughter self-conjugated
    # particles are if there are two charge-conjugated daughters
    ma.reconstructDecay("tau+:KS0 -> pi+ K_S0:merged", "", dmID=0, path=pa)
    ma.reconstructDecay("tau+:pi0 -> pi+ pi0:all", "", dmID=1, path=pa)
    # the pi0 is the daughter of the tau-
    ma.reconstructDecay("vpho:KS -> tau+:KS0 tau-:pi0", "", path=pa, chargeConjugation=False)
    # the pi0 is the daughter of the tau+
    ma.reconstructDecay("vpho:pi0 -> tau-:KS0 tau+:pi0", "", path=pa, chargeConjugation=False)
    # charge-conjugation has been turned off for the vpho lists above so the
    # merged list should just be the sum
    ma.copyLists("vpho:KSpi0", ["vpho:KS", "vpho:pi0"], path=pa)
    dump_3_v2nts(["KS", "pi0", "KSpi0"], path=pa)

    b2tu.safe_process(pa, 1)


class TestCopyLists(unittest.TestCase):
    """Analyse the output of ``run_copylists``"""

    def _count(self, listname):
        """Open the ntuple output and count the number of entries in the list."""
        df = root_pandas.read_root("ntuple.root", listname)
        return len(df)

    def test_merge_two_lists_with_identical_daughters(self):
        """Merging two lists with identical daughters should not double count."""
        self.assertEqual(self._count("a"), self._count("b"))
        self.assertEqual(self._count("a"), self._count("ab"))
        self.assertEqual(self._count("b"), self._count("ab"))

    def test_merge_two_lists_with_different_mdst_objects(self):
        """Merging two lists with different daughters but with the same mdst
        object should include each of them twice."""
        self.assertEqual(self._count("c"), self._count("d"))
        self.assertEqual(self._count("c") + self._count("d"), self._count("cd"))

    def test_different_daughter_order(self):
        """Merging two lists with daughters in a different order should not double count."""
        self.assertEqual(self._count("e"), self._count("e"))
        self.assertEqual(self._count("e"), self._count("ef"))

    def test_neutrals_in_decays_to_charge_conjugated_daughters(self):
        """Self-conjugated particles can be daughters of both charge-conjugated mother particles.
        Those multiple candidates are no duplicates and copying the particle list should not remove them."""
        self.assertEqual(self._count("g") + self._count("h"), self._count("gh"))

    def test_different_association_of_neutrals_same_number_of_daughters(self):
        """Different self-conjugated particles are daughters of charge-conjugated mother particles.
        The two mother particles have the same amount of daughters."""
        self.assertEqual(self._count("KS") + self._count("pi0"), self._count("KSpi0"))


if __name__ == "__main__":
    with b2tu.clean_working_directory():
        run_copylists()
        unittest.main()
