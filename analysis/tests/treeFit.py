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

        inputMdst('default', Belle2.FileSystem.findFile('analysis/tests/100_noBKG_B0ToPiPiPi0.root'), path=main)

        fillParticleList('pi+:a', 'pionID > 0.5', path=main)

        fillParticleList('gamma:a', '', path=main)
        reconstructDecay('pi0:a -> gamma:a gamma:a', '0.125 < InvM < 0.145', 0, path=main)

        reconstructDecay('B0:rec -> pi-:a pi+:a pi0:a', '', 0, path=main)
        matchMCTruth('B0:rec', path=main)

        vertexTree('B0:rec', conf_level=-1, ipConstraint=True, updateAllDaughters=True, path=main)

        ntupler = register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['chiProb', 'M', 'isSignal'])
        ntupler.param('particleList', 'B0:rec')
        main.add_module(ntupler)

        process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        truePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal > 0)")
        falsePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal == 0)")

        print("True fit survivors: {0} out of {1} true candidates".format(truePositives, allSig))
        print("False fit survivors: {0} out of {1} false candidates".format(falsePositives, allBkg))

        self.assertFalse(truePositives == 0, "No signal survived the fit.")

        self.assertFalse(falsePositives == 0, "No background survived the fit. This is weird.")

        self.assertTrue(truePositives > (allSig / 2.), "More than 50% of signal did not survived the fit.")

        self.assertFalse(allBkg == falsePositives, "All background candidates survived the fit.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
