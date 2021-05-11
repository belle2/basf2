#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import tempfile
import basf2
import modularAnalysis as ma
from vertex import treeFit
import b2test_utils
from ROOT import TFile


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        # we want to use the latest grated globaltag, not the old one from the
        # file
        basf2.conditions.disable_globaltag_replay()

        main = basf2.create_path()

        ma.inputMdst('default', b2test_utils.require_file('analysis/tests/100_noBKG_B0ToPiPiPi0.root'), path=main)

        ma.fillParticleList('pi+:a', 'pionID > 0.5', path=main)

        ma.fillParticleList('gamma:a', '', path=main)
        ma.reconstructDecay('pi0:a -> gamma:a gamma:a', '0.125 < InvM < 0.145', 0, path=main)

        ma.reconstructDecay('B0:rec -> pi-:a pi+:a pi0:a', '', 0, path=main)
        ma.matchMCTruth('B0:rec', path=main)

        treeFit('B0:rec', conf_level=-1, ipConstraint=False, updateAllDaughters=True, path=main)

        ntupler = basf2.register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['chiProb', 'M', 'isSignal'])
        ntupler.param('particleList', 'B0:rec')
        main.add_module(ntupler)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        truePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal > 0)")
        falsePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal == 0)")

        print(f"True fit survivors: {truePositives} out of {allSig} true candidates")
        print(f"False fit survivors: {falsePositives} out of {allBkg} false candidates")

        self.assertTrue(truePositives > 32, f"Signal rejection too high. True positives: {truePositives}")

        self.assertTrue(falsePositives < 2129, f"Background rejection got worse. False positives: {falsePositives}")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
