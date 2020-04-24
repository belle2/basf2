#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import root_pandas
import basf2 as b2
import b2test_utils as b2tu
import modularAnalysis as ma
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


if __name__ == "__main__":
    with b2tu.clean_working_directory():
        run_copylists()
        unittest.main()
