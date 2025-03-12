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
import b2test_utils

import basf2
import modularAnalysis as ma


class TestParticleLoader(unittest.TestCase):
    """Test case of ParticleLoader"""

    def testNumberOfParticles(self):
        """Compare the number of mdst objects and particles."""
        from ROOT import TFile

        testFile = tempfile.NamedTemporaryFile()

        main = basf2.create_path()

        ma.inputMdstList(filelist=[b2test_utils.require_file('analysis/1000_B_Jpsi_ks_pipi.root', 'validation')], path=main)

        ma.fillParticleList('pi+:all', '', path=main)
        ma.fillParticleList('gamma:all', '', path=main)
        ma.fillParticleList('K_L0:all', '', path=main)
        ma.fillParticleList('K_S0:V0 -> pi+ pi-', '', path=main)
        ma.fillParticleList('Lambda0:V0 -> p+ pi-', '', path=main)
        ma.fillConvertedPhotonsList('gamma:V0 -> e+ e-', '', path=main)

        ma.cutAndCopyList('K_L0:ecl', 'K_L0:all', 'isFromECL', path=main)
        ma.cutAndCopyList('K_L0:klm', 'K_L0:all', 'isFromKLM', path=main)

        ma.variablesToNtuple('',
                             ['nTrackFitResults(pi+)',
                              'nParticlesInList(pi+:all)',
                              'nNeutralECLClusters(1)',
                              'nParticlesInList(gamma:all)',
                              'nNeutralECLClusters(2)',
                              'nParticlesInList(K_L0:ecl)',
                              'nKLMClusters',
                              'nParticlesInList(K_L0:klm)',
                              'nValidV0s',
                              'nParticlesInList(K_S0:V0)',
                              'nParticlesInList(Lambda0:V0)',
                              'nParticlesInList(gamma:V0)'],
                             filename=testFile.name,
                             treename='tree',
                             path=main)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('tree')
        totalEvents = ntuple.GetEntries()
        self.assertFalse(totalEvents == 0, "Ntuple is empty.")

        missedTracks = ntuple.GetEntries("nTrackFitResults__bopi__pl__bc > nParticlesInList__bopi__pl__clall__bc")
        missedECLClustersNPhotons = ntuple.GetEntries("nNeutralECLClusters__bo1__bc > nParticlesInList__bogamma__clall__bc")
        missedECLClustersNeutralHad = ntuple.GetEntries("nNeutralECLClusters__bo2__bc > nParticlesInList__boK_L0__clecl__bc")
        missedKLMClusters = ntuple.GetEntries("nKLMClusters > nParticlesInList__boK_L0__clklm__bc")
        missedV0s = ntuple.GetEntries("nValidV0s > nParticlesInList__boK_S0__clV0__bc + nParticlesInList__boLambda0__clV0__bc + " +
                                      "nParticlesInList__bogamma__clV0__bc")

        self.assertTrue(missedTracks == 0, "Tracks are missed to be loaded as charged particles")
        self.assertTrue(missedECLClustersNPhotons == 0, "ECLClusters (nPhotons) are missed to be loaded as gammas")
        self.assertTrue(missedECLClustersNeutralHad == 0, "ECLClusters (neutralHadron) are missed to be loaded as K_L0s")
        self.assertTrue(missedKLMClusters == 0, "KLMClusters are missed to be loaded as K_L0s")
        self.assertTrue(missedV0s == 0, "V0s are missed to be loaded as K_S0, Lambda0, and converted-gamma")

        duplicatedTracks = ntuple.GetEntries("nTrackFitResults__bopi__pl__bc < nParticlesInList__bopi__pl__clall__bc")
        duplicatedECLClustersNPhotons = ntuple.GetEntries("nNeutralECLClusters__bo1__bc < nParticlesInList__bogamma__clall__bc")
        duplicatedECLClustersNeutralHad = ntuple.GetEntries("nNeutralECLClusters__bo2__bc < nParticlesInList__boK_L0__clecl__bc")
        duplicatedKLMClusters = ntuple.GetEntries("nKLMClusters < nParticlesInList__boK_L0__clklm__bc")
        duplicatedV0s = ntuple.GetEntries(
            "nValidV0s < nParticlesInList__boK_S0__clV0__bc + nParticlesInList__boLambda0__clV0__bc + " +
            "nParticlesInList__bogamma__clV0__bc")

        self.assertTrue(duplicatedTracks == 0, "Tracks are loaded as charged particles more than once")
        self.assertTrue(duplicatedECLClustersNPhotons == 0, "ECLClusters (nPhotons) are loaded as gammas more than once")
        self.assertTrue(duplicatedECLClustersNeutralHad == 0, "ECLClusters (neutralHadron) are loaded as K_L0s more than once")
        self.assertTrue(duplicatedKLMClusters == 0, "KLMClusters are loaded as K_L0s more than once")
        self.assertTrue(duplicatedV0s == 0, "V0s are loaded as K_S0, Lambda0, and converted-gamma more than once")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
