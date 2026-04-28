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


class TestTreeFits(unittest.TestCase):
    """The unit test case for TreeFitter"""

    def testFit(self):
        """Run the test fit"""

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        inputfile = b2test_utils.require_file('JPsi2ee_e2egamma.root', 'examples', py_case=self)
        ma.inputMdst(inputfile, path=main)

        ma.fillParticleList('e+:a', 'electronID > 0.5', path=main)
        ma.fillParticleList('gamma:a', '0.05 < E < 1.0', path=main)
        ma.correctBremsBelle('e+:corrected', 'e+:a', 'gamma:a', path=main)

        ma.reconstructDecay('J/psi:corrected -> e+:corrected e-:corrected ?addbrems', '', path=main)
        ma.matchMCTruth('J/psi:corrected', path=main)

        ma.applyCuts('J/psi:corrected',
                     'daughterSumOf(extraInfo(bremsCorrected)) == 2 and isSignal==1',
                     path=main)
        ma.applyEventCuts('nParticlesInList(J/psi:corrected)', path=main)

        conf = -1
        main.add_module('TreeFitter',
                        particleList='J/psi:corrected',
                        confidenceLevel=conf,
                        updateAllDaughters=True)

        ma.reconstructDecay('J/psi:no -> e+:a e-:a', '', path=main)
        ma.matchMCTruth('J/psi:no', path=main)
        ma.applyCuts('J/psi:no', 'isSignal==1', path=main)
        main.add_module('TreeFitter',
                        particleList='J/psi:no',
                        confidenceLevel=conf,
                        updateAllDaughters=True)

        ma.variablesToEventExtraInfo('J/psi:no', {'chiProb': 'chiProb_woCorrection', 'dM': 'dM_woCorrection'}, path=main)
        vm.addAlias('chiProb_woCorrection', 'eventExtraInfo(chiProb_woCorrection)')
        vm.addAlias('dM_woCorrection', 'eventExtraInfo(dM_woCorrection)')

        """
        alias_list = []
        xyz_error = ['x_uncertainty', 'y_uncertainty', 'z_uncertainty']
        for v in xyz_error:
            vm.addAlias(f'e0_diff_{v}', f'formula(daughter(0, {v}) - daughter(0, daughter(0, {v})))')
            vm.addAlias(f'e1_diff_{v}', f'formula(daughter(1, {v}) - daughter(1, daughter(0, {v})))')
            vm.addAlias(f'e0_pull_{v}', f'formula(e0_diff_{v} / daughter(0, {v}))')
            vm.addAlias(f'e1_pull_{v}', f'formula(e1_diff_{v} / daughter(1, {v}))')
            alias_list += [f'e0_pull_{v}', f'e1_pull_{v}']
        """

        ntupler = basf2.register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables',
                      ['chiProb', 'chiProb_woCorrection', 'dM', 'dM_woCorrection'])
        ntupler.param('particleList', 'J/psi:corrected')
        main.add_module(ntupler)

        basf2.process(main, 10000)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        dMImproved = ntuple.GetEntries("abs(dM) < abs(dM_woCorrection)")

        chiProbImproved = ntuple.GetEntries("(chiProb > chiProb_woCorrection) && (abs(dM) < abs(dM_woCorrection))")
        chiProbWorsened = ntuple.GetEntries("(chiProb < chiProb_woCorrection) && (abs(dM) < abs(dM_woCorrection))")

        self.assertTrue(dMImproved == 229, "Brems-correction algorithm or setting have been changed.")

        print(f"Vertex fit quality improved due to the brems-correction, # of candidates = {chiProbImproved}")
        print(f"Vertex fit quality worsened due to the brems-correction, # of candidates = {chiProbWorsened}")

        self.assertTrue(chiProbImproved == dMImproved, "Brems-correction causes negative effect on the vertex-fit.")
        print("Test passed, cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
