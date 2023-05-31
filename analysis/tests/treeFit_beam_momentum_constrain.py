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
from variables import variables as vm
from ROOT import TFile


basf2.set_random_seed('aSeed')


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        ma.inputMdst(b2test_utils.require_file('analysis/tests/150_noBKG_DtoPiNuNu.root'), path=main)
        ma.fillParticleList('pi+:a', 'pionID > 0.5', path=main)
        ma.fillParticleList('K+:a', 'kaonID > 0.5', path=main)

        ma.reconstructDecay(
            "D-:tag -> K+:a pi-:a pi-:a",
            cut='1.859 < M < 1.879 and useCMSFrame(p)>2.0',
            path=main
        )

        ma.reconstructDecay(
            "D+:sig -> pi+:a",
            '',
            path=main
        )

        ma.reconstructDecay('Z0:rec -> D+:sig D-:tag pi-:a pi+:a', '', path=main)
        ma.matchMCTruth('Z0:rec', path=main)

        ma.setBeamConstrainedMomentum(
            'Z0:rec',
            '^Z0:rec -> D+:sig D-:tag pi-:a pi+:a',
            'Z0:rec -> D+:sig D-:tag pi-:a pi+:a',
            path=main
            )

        ma.setBeamConstrainedMomentum(
           'Z0:rec',
           'Z0:rec -> ^D+:sig D-:tag pi-:a pi+:a',
           'Z0:rec -> D+:sig ^D-:tag ^pi-:a ^pi+:a',
           path=main
           )

        conf = 0
        main.add_module('TreeFitter',
                        particleList='Z0:rec',
                        confidenceLevel=conf,
                        massConstraintList=[],
                        massConstraintListParticlename=[],
                        expertBeamConstraintPDG=23,
                        treatAsInvisible='Z0:rec -> ^D+:sig D-:tag pi-:a pi+:a',
                        ipConstraint=True,
                        updateAllDaughters=True)

        ma.applyCuts('Z0:rec',
                     "1.6<daughter(0,M)<2.2",
                     path=main
                     )

        ma.rankByHighest("Z0:rec",
                         "chiProb",
                         numBest=1,
                         path=main)

        vm.addAlias("signalD", "daughter(0,isSignalAcceptMissingNeutrino)")

        ntupler = basf2.register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['chiProb', 'M', 'signalD'])
        ntupler.param('particleList', 'Z0:rec')
        main.add_module(ntupler)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("signalD == 0")
        allSig = ntuple.GetEntries("signalD > 0")

        truePositives = ntuple.GetEntries("(chiProb > 0) && (signalD > 0)")
        falsePositives = ntuple.GetEntries("(chiProb > 0) && (signalD == 0)")

        mustBeZero = ntuple.GetEntries(f"(chiProb < {conf})")

        print(f"True fit survivors: {truePositives} out of {allSig} true candidates")
        print(f"False fit survivors: {falsePositives} out of {allBkg} false candidates")

        self.assertFalse(truePositives == 0, "No signal survived the fit.")

        self.assertTrue(falsePositives < 1, "Background rejection too small.")

        self.assertTrue(truePositives > 8, "Signal rejection too high")
        self.assertFalse(mustBeZero, f"We should have dropped all candidates with confidence level less than {conf}.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
