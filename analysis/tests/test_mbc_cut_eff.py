#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import os
import tempfile
import basf2
import b2test_utils
import modularAnalysis as ma
from ROOT import Belle2
from ROOT import TFile
from ROOT import TNtuple


class TestMbcEff(unittest.TestCase):
    """The unit test"""

    def testMbcEff(self):
        """Reconstruct stuff with tight Mbc cut."""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        inputfile = b2test_utils.require_file(
            'analysis/1000_B_DstD0Kpi_skimmed.root', 'validation', py_case=self)
        ma.inputMdst('default', inputfile, path=main)

        ma.fillParticleList('pi+:a', 'pionID > 0.5', path=main)
        ma.fillParticleList('K+:a', 'kaonID > 0.5', path=main)

        ma.reconstructDecay('D0:rec -> K-:a pi+:a', '', 0, path=main)
        ma.reconstructDecay('D*+:rec -> D0:rec pi+:a', '', 0, path=main)
        ma.reconstructDecay('B0:rec -> D*+:rec pi-:a', ' InvM > 5.27', 0, path=main)
        ma.matchMCTruth('B0:rec', path=main)

        ntupler = basf2.register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['isSignal'])
        ntupler.param('particleList', 'B0:rec')
        main.add_module(ntupler)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        print("True candidates {0}".format(allSig))
        print("False candidates {0}".format(allBkg))

        sig_expected = 136
        bkg_expected = 1630

        self.assertTrue(
            allSig == sig_expected,
            f"Mbc cut efficency has changed! n_sig expected: {sig_expected} found: {sig_expected}.")
        self.assertTrue(allBkg == bkg_expected, f"Mbc cut background has changed! n_bkg expected: {bkg_expected} found: {allBkg}.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
