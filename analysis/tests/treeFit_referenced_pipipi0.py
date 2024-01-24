#!/usr/bin/env python3

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


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        inputfile = b2test_utils.require_file('analysis/B0ToPiPiPi0.root', 'validation', py_case=self)
        ma.inputMdst(inputfile, path=main)

        ma.fillParticleList('pi+:a', 'pionID > 0.5', path=main)

        ma.fillParticleList('gamma:a', '', path=main)
        ma.reconstructDecay('pi0:a -> gamma:a gamma:a', '0.125 < InvM < 0.145', 0, path=main)

        ma.reconstructDecay('B0:rec -> pi-:a pi+:a pi0:a', '', 0, path=main)
        ma.matchMCTruth('B0:rec', path=main)

        conf = 0
        main.add_module('TreeFitter',
                        particleList='B0:rec',
                        confidenceLevel=conf,
                        massConstraintList=[],
                        massConstraintListParticlename=[],
                        expertUseReferencing=True,
                        ipConstraint=False,
                        updateAllDaughters=True)

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

        mustBeZero = ntuple.GetEntries(f"(chiProb < {conf})")

        print(f"True fit survivors: {truePositives} out of {allSig} true candidates")
        print(f"False fit survivors: {falsePositives} out of {allBkg} false candidates")

        self.assertFalse(truePositives == 0, "No signal survived the fit.")

        self.assertTrue(falsePositives < 8299, "Background rejection increased.")

        self.assertTrue(truePositives > 212, "Signal rejection too high")
        self.assertFalse(mustBeZero, f"We should have dropped all candidates with confidence level less than {conf}.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
