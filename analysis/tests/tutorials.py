#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from ROOT import Belle2
import unittest
import glob


def test_one_tutorial(tutorial_name):
    loc = Belle2.FileSystem.findFile('analysis/examples/tutorials/' + tutorial_name)
    return os.system('basf2 ' + loc + ' -n 1')


class TutorialsTest(unittest.TestCase):

    supported_tutorials = ['B2A101-Y4SEventGeneration.py',
                           'B2A102-SimulateAndReconstruct.py',
                           'B2A201-LoadMCParticles.py',
                           'B2A202-LoadReconstructedParticles.py',
                           'B2A203-LoadV0s.py',
                           'B2A204-LoadAllECLClusters.py',
                           'B2A301-Dstar2D0Pi-Reconstruction.py',
                           'B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.py',
                           'B2A303-MultipleDecays-Reconstruction.py',
                           'B2A304-B02RhoGamma-Reconstruction.py',
                           'B2A305-Btag+SingleMuon-Reconstruction.py',
                           'B2A306-B02RhoGamma-withPi0EtaVeto.py',
                           'B2A307-BasicEventWiseNtupleSelection.py',
                           'B2A400-TreeFit.py',
                           'B2A501-FSRCorrection.py',
                           'B2A502-WriteOutDecayHash.py',
                           'B2A503-ReadDecayHash.py',
                           'B2A601-ParticleStats.py',
                           'B2A602-BestCandidateSelection.py',
                           'B2A701-ContinuumSuppression_Input.py',
                           'B2A702-ContinuumSuppression_MVATrain.py',
                           'B2A703-ContinuumSuppression_MVAExpert.py',
                           'B2A704-EventShape.py',
                           'B2A801-FlavorTagger.py',
                           'B2A901-PrintOutDataStore.py',
                           'B2A902-PrintOutMCParticles.py',
                           'B2A903-PrintOutVariableValues.py',
                           'B2A904-LookUpTableCreation.py',
                           'B2A905-ApplyWeightsToTracks.py']

    broken_tutorials = ['B2A701-ContinuumSuppression_Input.py',  # BII-4246
                        'B2A702-ContinuumSuppression_MVATrain.py',  # BII-4246
                        'B2A703-ContinuumSuppression_MVAExpert.py',  # BII-4246
                        'B2A801-FlavorTagger.py'  # BII-4247
                        ]

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
    def test_tutorials(self):
        """
        Test supported tutorials.
        """
        for t in self.supported_tutorials:
            with self.subTest(t):
                if t in self.broken_tutorials:
                    pass
                else:
                    self.assertEqual(test_one_tutorial(t), 0)

    @unittest.skipIf(not os.getenv('BELLE2_LOCAL_DIR'),
                     "$BELLE2_LOCAL_DIR not found.")
    def test_no_uncovered_tutorials(self):
        """
        Check that anbalysis/examples/tutorials/ doesn't containt tutorials
        that are not covered by the test.
        """
        available_tutorials = [x.split("/")[-1]
                               for x in glob.glob(os.getenv('BELLE2_LOCAL_DIR') + "/analysis/examples/tutorials/*.py")]
        self.assertEqual(set(available_tutorials), set(self.supported_tutorials))


if __name__ == '__main__':
    unittest.main()
