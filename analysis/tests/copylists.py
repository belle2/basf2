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
import uproot
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
    ma.inputMdst(b2.find_file("analysis/tests/mdst.root"), path=pa)
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

    # third test: check that two lists with the same daughters in different
    # orders don't double count
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

    # sixth test: check that check of mother flavor can be turned off when identifying duplicates
    ma.reconstructDecay("D0 -> K- pi+", "", dmID=0, chargeConjugation=False, path=pa)
    ma.matchMCTruth("D0", path=pa)
    ma.cutAndCopyList("D0:sig", "D0", "isSignal==1", path=pa)
    ma.reconstructDecay("anti-D0:anti -> K- pi+", "", dmID=1, chargeConjugation=False, path=pa)
    ma.matchMCTruth("anti-D0:anti", path=pa)
    ma.cutAndCopyList("anti-D0:antisig", "anti-D0:anti", "isSignal==1", path=pa)
    ma.mergeListsWithBestDuplicate("D0:ignore", ['D0', 'anti-D0:anti'], 'isSignal', path=pa,
                                   preferLowest=False, ignoreMotherFlavor=True)
    ma.copyLists("D0:flavor", ['D0', 'anti-D0:anti'], path=pa)
    ma.variablesToNtuple("D0",  ['isSignal', 'PDG'], treename="D0", path=pa)
    ma.variablesToNtuple("anti-D0:anti", ['isSignal', 'PDG'], treename="anti", path=pa)
    ma.variablesToNtuple("D0:ignore", ['isSignal', 'PDG'], treename="ignore", path=pa)
    ma.variablesToNtuple("D0:flavor", ['isSignal', 'PDG'], treename="flavor", path=pa)

    b2tu.safe_process(pa, 1)


class TestCopyLists(unittest.TestCase):
    """Analyse the output of ``run_copylists``"""

    def _count(self, listname):
        """Open the ntuple output and count the number of entries in the list."""
        df = uproot.open("ntuple.root")[listname].arrays(library="pd")
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

    def test_ignore_mother_flavor(self):
        """Particle lists can be merged ignoring the flavor of the mother particle. If both particle
        lists decay into the same final state, the size of the combined list is identical to one
        individual list. Otherwise, it's the sum of the two."""
        self.assertEqual(self._count("D0"), self._count("ignore"))
        self.assertEqual(self._count("D0") + self._count("anti"), self._count("flavor"))


if __name__ == "__main__":
    with b2tu.clean_working_directory():
        run_copylists()
        unittest.main()
