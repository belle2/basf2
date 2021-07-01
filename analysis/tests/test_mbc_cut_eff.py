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
import tempfile
import basf2
import b2test_utils
import modularAnalysis as ma
from ROOT import TFile


class TestMbcEff(unittest.TestCase):
    """The unit test"""

    def testMbcEff(self):
        """Reconstruct stuff with tight Mbc cut."""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        inputfile = b2test_utils.require_file(
            'analysis/1000_B_DstD0Kpi_skimmed.root', 'validation', py_case=self)
        ma.inputMdst('default', inputfile, path=main)

        ma.fillParticleList('pi+:a', 'pidProbabilityExpert(211, ALL) > 0.5', path=main)
        ma.fillParticleList('K+:a', 'pidProbabilityExpert(321, ALL) > 0.5', path=main)

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

        print(f"True candidates {allSig}")
        print(f"False candidates {allBkg}")

        sig_expected = 136
        bkg_expected = 1630

        self.assertTrue(
            allSig == sig_expected,
            f"Mbc cut efficency has changed! n_sig expected: {sig_expected} found: {allSig}.")
        self.assertTrue(allBkg == bkg_expected, f"Mbc cut background has changed! n_bkg expected: {bkg_expected} found: {allBkg}.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
