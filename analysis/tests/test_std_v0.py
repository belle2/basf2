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

from basf2 import create_path, process
import modularAnalysis as ma
import stdV0s
from ROOT import TFile


class TestStdV0(unittest.TestCase):
    """Test case for standard V0 lists"""

    def _check_list(
            self,
            std_function=stdV0s.stdKshorts,
            expected_modules=[
                "ParticleLoader",
                "ParticleListManipulator",
                "TreeFitter",
                "ParticleSelector",
                "ParticleLoader",
                "ParticleCombiner",
                "TreeFitter",
                "ParticleSelector",
                "ParticleListManipulator"],
            expected_lists=['V0_ToFit', 'V0_ToFit', 'all', 'RD', 'merged']):
        """check that a given function works"""
        testpath = create_path()
        std_function(path=testpath)

        loaded_modules = []
        built_lists = []
        for module in testpath.modules():
            loaded_modules.append(module.type())
            for param in module.available_params():
                if module.type() == 'ParticleLoader' and param.name == 'decayStrings':
                    name = param.values[0].split(':')[1].split(' -> ')[0]
                    built_lists.append(name)
                if module.type() == 'ParticleListManipulator' and param.name == 'outputListName':
                    name = str(param.values).split(':')[1].split(' -> ')[0]
                    built_lists.append(name)
                if module.type() == 'ParticleCombiner' and param.name == 'decayString':
                    name = param.values.split(':')[1].split(' -> ')[0]
                    built_lists.append(name)

        # Check that we load the correct modules
        self.assertListEqual(
            loaded_modules, expected_modules,
            f"Loaded modules do not match the expected ones (function {std_function.__name__})")

        # Check that we load the correct particle lists
        self.assertListEqual(
            built_lists, expected_lists,
            f"Built particles lists do not match the expected ones (function {std_function.__name__})")

    def test_stdkshorts_list(self):
        """check that the builder function works with the stdKshorts list"""
        self._check_list()

    def test_belle_list(self):
        """check that the builder function works with the legacy Belle Kshorts list"""
        expected_modules = ["ParticleLoader",
                            "ParticleListManipulator",
                            "ParticleSelector",
                            "ParticleVertexFitter",
                            "ParticleSelector"]
        self._check_list(std_function=stdV0s.goodBelleKshort, expected_modules=expected_modules,
                         expected_lists=["legacyGoodKS", "legacyGoodKS"])

    def test_stdlambdas_list(self):
        """check that the builder function works with the stdLambdas list"""
        expected_modules = ["ParticleLoader",
                            "ParticleListManipulator",
                            "TreeFitter",
                            "ParticleSelector",
                            "DuplicateVertexMarker",
                            "ParticleSelector",
                            "ParticleLoader",
                            "ParticleLoader",
                            "ParticleCombiner",
                            "TreeFitter",
                            "ParticleSelector",
                            "DuplicateVertexMarker",
                            "ParticleSelector",
                            "ParticleListManipulator"]
        expected_lists = ['V0_ToFit', 'V0_ToFit', 'all', 'all', 'RD', 'merged']
        self._check_list(std_function=stdV0s.stdLambdas, expected_modules=expected_modules, expected_lists=expected_lists)

    def test_kshort_signals(self):
        """check the number of signals in K_S0:merged and K_S0:scaled lists"""

        main = create_path()

        inputfile = b2test_utils.require_file('analysis/1000_B_Jpsi_ks_pipi.root', 'validation', py_case=self)
        ma.inputMdst(inputfile, path=main)

        stdV0s.stdKshorts(path=main)  # -> K_S0:merged
        stdV0s.scaleErrorKshorts(path=main)  # -> K_S0:scaled

        ma.matchMCTruth('K_S0:merged', path=main)
        ma.matchMCTruth('K_S0:scaled', path=main)

        testFile = tempfile.NamedTemporaryFile()
        ma.variablesToNtuple('K_S0:merged', ['isSignal', 'M'], filename=testFile.name, treename='merged', path=main)
        ma.variablesToNtuple('K_S0:scaled', ['isSignal', 'M'], filename=testFile.name, treename='scaled', path=main)

        process(main)

        ntuplefile = TFile(testFile.name)
        ntuple_merged = ntuplefile.Get('merged')
        ntuple_scaled = ntuplefile.Get('scaled')

        allSig_merged = ntuple_merged.GetEntries("isSignal == 1")
        allSig_scaled = ntuple_scaled.GetEntries("isSignal == 1")

        print(f"Number of signal K_S0:merged: {allSig_merged}")
        print(f"Number of signal K_S0:scaled: {allSig_scaled}")

        self.assertTrue(allSig_merged > 999,  "Number of signal K_S0:merged is too small.")
        self.assertTrue(allSig_scaled > 1000, "Number of signal K_S0:scaled is too small.")

        tightMSig_merged = ntuple_merged.GetEntries("isSignal == 1 && M > 0.48 && M < 0.52")
        tightMSig_scaled = ntuple_scaled.GetEntries("isSignal == 1 && M > 0.48 && M < 0.52")

        print(f"Number of signal K_S0:merged with 0.48<M<0.52: {tightMSig_merged}")
        print(f"Number of signal K_S0:scaled with 0.48<M<0.52: {tightMSig_scaled}")

        self.assertTrue(tightMSig_merged > 962, "Number of signal K_S0:merged with 0.48<M<0.52 is too small.")
        self.assertTrue(tightMSig_scaled > 962, "Number of signal K_S0:scaled with 0.48<M<0.52  is too small.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    unittest.main()
