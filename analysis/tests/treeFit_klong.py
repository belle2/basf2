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

        basf2.set_random_seed('klong')
        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        ma.inputMdstList([basf2.find_file('B02JpsiKL_Jpsi2mumu.root', 'examples', False)], path=main)

        ma.fillParticleList('mu+:sig', 'muonID > 0.5', path=main)
        stdKlongs('allklm', path=main)

        ma.reconstructDecay('J/psi:mumu -> mu-:sig mu+:sig', '3.08 < M < 3.12', path=main)
        ma.reconstructDecay('B0:sig -> J/psi:mumu K_L0:allklm', '', path=main)
        ma.matchMCTruth('B0:sig', path=main)

        conf = -1
        main.add_module('TreeFitter',
                        particleList='B0:sig',
                        confidenceLevel=conf,
                        massConstraintList=[511],
                        ipConstraint=True,
                        updateAllDaughters=True)

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

        SigDeltaEReasonable = ntuple.GetEntries("isSignal==1 && deltaE<0.1 && chiProb>0")

        print(f"True fit survivors: {truePositives} out of {allSig} true candidates")
        print(f"False fit survivors: {falsePositives} out of {allBkg} false candidates")
        print(f"True fit survivors with reasonable deltaE: {SigDeltaEReasonable}")

        self.assertTrue(allBkg - falsePositives >= 37,
                        f"Background rejection: {allBkg-falsePositives} out of {allBkg} false candidates rejected")
        self.assertTrue(truePositives >= 508, f"Signal efficiency: {truePositives} out of {allSig} true candidates retained")

        self.assertTrue(SigDeltaEReasonable >= 433,
                        f"Signal kinematics is correctly reconstructed in {SigDeltaEReasonable} candidates.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
