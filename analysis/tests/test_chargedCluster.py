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
from basf2 import create_path, register_module
import b2test_utils
import modularAnalysis as ma
from variables import variables as vm

from ROOT import TFile


class TestChargedCluster(unittest.TestCase):
    """The unit test"""

    def test(self):
        """ Test of fillParticleListFromChargedCluster """
        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        inputfile = b2test_utils.require_file('mdst14.root', 'validation', py_case=self)
        ma.inputMdst(inputfile, path=main)

        ma.fillParticleList('pi+:ecl', 'clusterE>0', path=main)
        ma.fillParticleList('pi+:klm', 'klmClusterEnergy>0', path=main)

        ma.fillParticleListFromChargedCluster('gamma:bestecl', 'pi+:ecl', '',
                                              useOnlyMostEnergeticECLCluster=True, path=main)
        ma.fillParticleListFromChargedCluster('gamma:allecl', 'pi+:ecl', '',
                                              useOnlyMostEnergeticECLCluster=False, path=main)

        ma.fillParticleListFromChargedCluster('gamma:klmT', 'pi+:klm', '',
                                              useOnlyMostEnergeticECLCluster=True, path=main)
        ma.fillParticleListFromChargedCluster('gamma:klmF', 'pi+:klm', '',
                                              useOnlyMostEnergeticECLCluster=False, path=main)

        ma.fillParticleListFromChargedCluster('K_L0:ecl', 'pi+:ecl', '', path=main)
        ma.fillParticleListFromChargedCluster('K_L0:klm', 'pi+:klm', '', path=main)

        ma.applyCuts('gamma:bestecl', 'isFromECL', path=main)
        ma.applyCuts('gamma:allecl', 'isFromECL', path=main)
        ma.applyCuts('gamma:klmT', 'isFromKLM', path=main)
        ma.applyCuts('gamma:klmF', 'isFromKLM', path=main)
        ma.applyCuts('K_L0:ecl', 'isFromECL', path=main)
        ma.applyCuts('K_L0:klm', 'isFromKLM', path=main)

        vm.addAlias('n_pi_ecl', 'nParticlesInList(pi+:ecl)')
        vm.addAlias('n_pi_klm', 'nParticlesInList(pi+:klm)')
        vm.addAlias('n_gamma_bestecl', 'nParticlesInList(gamma:bestecl)')
        vm.addAlias('n_gamma_allecl', 'nParticlesInList(gamma:allecl)')
        vm.addAlias('n_gamma_klmT', 'nParticlesInList(gamma:klmT)')
        vm.addAlias('n_gamma_klmF', 'nParticlesInList(gamma:klmF)')
        vm.addAlias('n_KL_ecl', 'nParticlesInList(K_L0:ecl)')
        vm.addAlias('n_KL_klm', 'nParticlesInList(K_L0:klm)')

        ntupler = register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables',
                      ['n_pi_ecl', 'n_pi_klm',
                       'n_gamma_bestecl', 'n_gamma_allecl', 'n_gamma_klmT', 'n_gamma_klmF',
                       'n_KL_ecl', 'n_KL_klm'])
        main.add_module(ntupler)

        b2test_utils.safe_process(main, 1000)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        nEntries = ntuple.GetEntries()

        # charged cluster seems always having flag of N1 (n photon). Is it true?
        sameNumberForPiAndGamma_ECL = ntuple.GetEntries("n_pi_ecl == n_gamma_bestecl")
        # not the case for N2?
        sameNumberForPiAndHadron_ECL = ntuple.GetEntries("n_pi_ecl >= n_KL_ecl")
        # KLM cluster has no preference for particle type
        sameNumberForPiAndBothGammaHadron_KLM = ntuple.GetEntries("(n_pi_klm == n_gamma_klmT) && (n_pi_klm == n_KL_klm)")

        self.assertTrue(nEntries == sameNumberForPiAndGamma_ECL,
                        "Charged ECL-cluster is not correctly loaded for gamma.")
        self.assertTrue(nEntries == sameNumberForPiAndHadron_ECL,
                        "Charged ECL-cluster is not correctly loaded for neutral hadron.")
        self.assertTrue(nEntries == sameNumberForPiAndBothGammaHadron_KLM,
                        "Charged KLM-cluster is not correctly loaded for both gamma and neutral hadron.")

        allIsGreaterThanBest = ntuple.GetEntries("n_gamma_allecl >= n_gamma_bestecl")
        self.assertTrue(nEntries == allIsGreaterThanBest, "Charged ECL-cluster is missed with useOnlyMostEnergeticECLCluster=False")

        optionDoesNotChangeKLM = ntuple.GetEntries("n_gamma_klmT == n_gamma_klmF")
        self.assertTrue(nEntries == optionDoesNotChangeKLM, "Charged KLM-cluster is affected by useOnlyMostEnergeticECLCluster")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
