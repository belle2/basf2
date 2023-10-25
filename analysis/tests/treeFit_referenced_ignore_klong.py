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
from stdKlongs import stdKlongs
from ROOT import TFile


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        ma.inputMdstList([basf2.find_file('B02JpsiKL_Jpsi2mumu.root', 'examples', False)], path=main)

        ma.fillParticleList('mu+:sig', 'muonID > 0.5', path=main)
        stdKlongs('allklm', path=main)

        ma.reconstructDecay('J/psi:mumu -> mu-:sig mu+:sig', '3.08 < M < 3.12', path=main)
        ma.reconstructMissingKlongDecayExpert('B0:sig -> J/psi:mumu K_L0:allklm', '',
                                              path=main,
                                              recoList="_reco")
        ma.matchMCTruth('B0:sig', path=main)

        conf = -1
        main.add_module('TreeFitter',
                        particleList='B0:sig',
                        confidenceLevel=conf,
                        massConstraintList=[],
                        massConstraintListParticlename=[],
                        expertUseReferencing=True,
                        ipConstraint=True,
                        updateAllDaughters=True,
                        ignoreFromVertexFit='B0 -> J/psi ^K_L0')

        ntupler = basf2.register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['chiProb', 'isSignal', 'Mbc', 'deltaE', 'dz'])
        ntupler.param('particleList', 'B0:sig')
        main.add_module(ntupler)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allSig = ntuple.GetEntries("isSignal == 1")
        allBkg = ntuple.GetEntries("isSignal == 0")

        truePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal == 1)")
        falsePositives = ntuple.GetEntries("(chiProb > 0) && (isSignal == 0)")

        SigDeltaEReasonable = ntuple.GetEntries("isSignal==1 && deltaE<0.1")

        print(f"True fit survivors: {truePositives} out of {allSig} true candidates")
        print(f"False fit survivors: {falsePositives} out of {allBkg} false candidates")

        self.assertTrue(falsePositives < 2041, "Background rejection too small.")
        self.assertTrue(truePositives > 522, "Signal rejection too high")

        print(f"Signal events with good deltaE values {SigDeltaEReasonable}")
        self.assertTrue(SigDeltaEReasonable > 463, "Signal kinematics is wrongly reconstructed too much")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
