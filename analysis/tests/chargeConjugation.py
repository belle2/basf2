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
from variables import variables
from ROOT import TFile


class TestParticleCombiner(unittest.TestCase):
    """The unit test case for the ParticleCombinerModule"""

    def testChargeConjugation(self):
        """Reconstruct decays with and without charge conjugation"""

        testFile = tempfile.NamedTemporaryFile()
        # make logging more reproducible by replacing some strings.
        # Also make sure the testfile name is replaced if necessary
        b2test_utils.configure_logging_for_tests({testFile.name: "${testfile}"})

        main = basf2.create_path()
        ma.inputMdst('default', b2test_utils.require_file('analysis/tests/mdst.root'), path=main)

        ma.fillParticleList('pi+:pionlike', 'pionID > 0.5', path=main)
        ma.fillParticleList('K+:kaonlike', 'kaonID > 0.5', path=main)

        # combine all kaons with all pions for both charge configurations (K- pi+ and K+ pi-)
        ma.reconstructDecay('D0:all -> K-:kaonlike pi+:pionlike', '', chargeConjugation=True, path=main)

        # only reconstruct D0
        ma.reconstructDecay('D0:particle -> K-:kaonlike pi+:pionlike', '', chargeConjugation=False, path=main)

        # only reconstruct anti-D0
        ma.reconstructDecay('anti-D0:anti-particle -> K+:kaonlike pi-:pionlike', '', chargeConjugation=False, path=main)

        # use particle's charge to separate D0 flavors,
        ma.reconstructDecay(
            'D0:positiveFlavour -> K-:kaonlike pi+:pionlike',
            'daughter(0, charge) < 0',
            chargeConjugation=True,
            path=main)
        ma.reconstructDecay(
            'D0:negativeFlavour -> K-:kaonlike pi+:pionlike',
            'daughter(0, charge) > 0',
            chargeConjugation=True,
            path=main)

        variables.addAlias('nDs', 'nParticlesInList(D0:all)')
        variables.addAlias('nD0s', 'nParticlesInList(D0:particle)')
        variables.addAlias('nAntiD0s', 'nParticlesInList(anti-D0:anti-particle)')
        variables.addAlias('nPositiveFlavourDs', 'nParticlesInList(D0:positiveFlavour)')
        variables.addAlias('nNegativeFlavourDs', 'nParticlesInList(D0:negativeFlavour)')

        allvariables = ['nDs', 'nD0s', 'nAntiD0s', 'nPositiveFlavourDs', 'nNegativeFlavourDs']
        ma.variablesToNtuple('', variables=allvariables, filename=testFile.name, path=main)

        basf2.process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('variables')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        # sum of D0s and D0bars should be equal to all D0s
        if (ntuple.GetEntries() != ntuple.GetEntries("nDs == nD0s + nAntiD0s")):
            ntuple.Scan("nDs:nD0s:nAntiD0s:nPositiveFlavourDs:nNegativeFlavourDs", "", "", 10)
            self.assertFalse(True, "Number of D0s does not agree with sum of both flavors!")

        # D0s should be combinations of K- and pi+ only
        if (ntuple.GetEntries() != ntuple.GetEntries("nD0s == nPositiveFlavourDs")):
            ntuple.Scan("nDs:nD0s:nAntiD0s:nPositiveFlavourDs:nNegativeFlavourDs", "", "", 10)
            self.assertFalse(True, "Number of D0s does not agree with number of K- pi+ combinations!")

        # D0bars should be combinations of K+ and pi- only
        if (ntuple.GetEntries() != ntuple.GetEntries("nAntiD0s == nNegativeFlavourDs")):
            ntuple.Scan("nDs:nD0s:nAntiD0s:nPositiveFlavourDs:nNegativeFlavourDs", "", "", 10)
            self.assertFalse(True, "Number of anti D0s does not agree with number of K+ pi- combinations!")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
