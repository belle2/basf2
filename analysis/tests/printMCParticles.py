#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Tests for the ``storeCompact`` option of `modularAnalysis.printMCParticles`.
"""

import unittest
import basf2
import modularAnalysis as ma
import b2test_utils
from ROOT import Belle2

#: characters used by the default (non-compact) tree representation to draw the tree structure
TREE_DRAWING_CHARACTERS = '├╰│╶'


class CollectMCDecayStrings(basf2.Module):
    """Collect the value of the ``MCDecayString`` StoreObjPtr for every processed event."""

    def initialize(self):
        """reimplementation"""
        #: list of MCDecayString values, one per processed event
        self.strings = []

    def event(self):
        """reimplementation"""
        stringWrapper = Belle2.PyStoreObj('MCDecayString')
        if stringWrapper.isValid():
            self.strings.append(stringWrapper.obj().getString())


def collect(**kwargs):
    """
    Run `modularAnalysis.printMCParticles` with the given keyword arguments on a few
    events of the local test mdst file and return the list of resulting
    ``MCDecayString`` values, one per event.
    """

    main = basf2.create_path()
    ma.inputMdst(basf2.find_file('analysis/tests/mdst.root'), path=main)
    main.add_module('EventLimiter', maxEventsPerRun=5)
    ma.printMCParticles(path=main, suppressPrint=True, **kwargs)
    collector = CollectMCDecayStrings()
    main.add_module(collector)
    with b2test_utils.show_only_errors():
        basf2.process(main)
    return collector.strings


class TestPrintMCParticlesStoreCompact(unittest.TestCase):
    """Tests for the ``storeCompact`` option of the `PrintMCParticles` module"""

    def test_compact_differs_from_default_tree(self):
        """the compact string should be a single line and not use the default tree's indentation"""

        compact = collect(storeCompact=True)
        full = collect(storeCompact=False)

        self.assertTrue(len(compact) > 0, "no events processed")
        self.assertEqual(len(compact), len(full))

        for compactString, fullString in zip(compact, full):
            self.assertNotIn('\n', compactString, "compact string should be a single line")
            for boxChar in TREE_DRAWING_CHARACTERS:
                self.assertNotIn(boxChar, compactString,
                                 "compact string should not use the default tree's indentation characters")
            # sanity check that the full (default) string does still use the indented tree format,
            # so the difference above is actually meaningful
            self.assertIn('\n', fullString)
            self.assertTrue(any(boxChar in fullString for boxChar in TREE_DRAWING_CHARACTERS))
            self.assertLess(len(compactString), len(fullString))

    def test_max_level_truncates_compact_string(self):
        """with maxLevel=1 the compact string should never show a decay ('->')"""

        strings = collect(storeCompact=True, maxLevel=1)
        self.assertTrue(len(strings) > 0, "no events processed")
        for decayString in strings:
            self.assertNotIn('->', decayString)

    def test_only_primaries_filters_secondaries(self):
        """with onlyPrimaries=True the compact string should not mark any particle as secondary ('~')"""

        strings = collect(storeCompact=True, onlyPrimaries=True)
        self.assertTrue(len(strings) > 0, "no events processed")
        for decayString in strings:
            self.assertNotIn('~', decayString)


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
