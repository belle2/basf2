#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import os
import tempfile
from basf2 import *
from modularAnalysis import *
from vertex import vertexTree
from ROOT import Belle2
from ROOT import TFile
from ROOT import TNtuple


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        inputMdst('default', Belle2.FileSystem.findFile('analysis/1000_B_DstD0Kpi_skimmed.root', 'validation'), path=main)

        fillParticleList('pi+:a', 'pionID > 0.5', path=main)
        fillParticleList('K+:a', 'kaonID > 0.5', path=main)

        reconstructDecay('D0:rec -> K-:a pi+:a', '', 0, path=main)
        reconstructDecay('D*+:rec -> D0:rec pi+:a', '', 0, path=main)
        reconstructDecay('B0:rec -> D*+:rec pi-:a', ' InvM > 5.27', 0, path=main)
        matchMCTruth('B0:rec', path=main)

        ntupler = register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['isSignal'])
        ntupler.param('particleList', 'B0:rec')
        main.add_module(ntupler)

        process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        print("True candidates {0}".format(allSig))
        print("False candidates {0}".format(allBkg))

        sig_expected = 136
        bkg_expected = 1680

        self.assertTrue(
            allSig == sig_expected,
            f"Mbc cut efficency has changed! n_sig expected: {sig_expected} found: {sig_expected}.")
        self.assertTrue(allBkg == bkg_expected, f"Mbc cut background hhas changed! n_bkg expected: {bkg_expected} found: {allBkg}.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
