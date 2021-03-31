#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import tempfile
from basf2 import create_path, register_module
import b2test_utils
from modularAnalysis import fillParticleListFromMC, reconstructMCDecay, inputMdst
from ROOT import TFile


class TestNewMCDecayFinder(unittest.TestCase):
    """The unit test"""

    def testNewMCDecayFinder(self):
        """Reconstruct/search for an MC decay chain using the reconstructMCDecay tool."""

        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        inputfile = b2test_utils.require_file(
            'analysis/1000_B_DstD0Kpi_skimmed.root', 'validation', py_case=self)
        inputMdst('default', inputfile, path=main)

        fillParticleListFromMC('gamma:primaryMC', 'mcPrimary', path=main)
        fillParticleListFromMC('K+:primaryMC', 'mcPrimary', path=main)
        fillParticleListFromMC('pi+:primaryMC', 'mcPrimary', path=main)

        reconstructMCDecay(
            'B0:DstD0Kpi =direct=> [D*+:MC =direct=> [D0:MC =direct=> K-:primaryMC pi+:primaryMC ] pi+:primaryMC] pi-:primaryMC',
            '',
            path=main)

        ntupler = register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['isSignal'])
        ntupler.param('particleList', 'B0:DstD0Kpi')
        main.add_module(ntupler)

        b2test_utils.safe_process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        print("True candidates {0}".format(allSig))
        print("False candidates {0}".format(allBkg))

        sig_expected = 406

        self.assertTrue(
            allSig == sig_expected,
            f"n_sig expected: {sig_expected} found: {sig_expected}.")
        self.assertTrue(allBkg == 0, f"n_bkg expected 0, found: {allBkg}.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
