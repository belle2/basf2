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
from basf2 import create_path, register_module
import b2test_utils
from modularAnalysis import fillParticleListFromMC, reconstructMCDecay, inputMdst, findMCDecay
from ROOT import TFile


class TestNewMCDecayFinder(unittest.TestCase):
    """The unit test"""

    def testReconstructMCDecay(self):
        """Reconstruct/search for an MC decay chain using the reconstructMCDecay tool."""

        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        inputfile = b2test_utils.require_file(
            'analysis/1000_B_DstD0Kpi_skimmed.root', 'validation', py_case=self)
        inputMdst(inputfile, path=main)

        fillParticleListFromMC('gamma:primaryMC', 'mcPrimary', path=main)
        fillParticleListFromMC('K+:primaryMC', 'mcPrimary', path=main)
        fillParticleListFromMC('pi+:primaryMC', 'mcPrimary', path=main)

        reconstructMCDecay(
            'B0:DstD0Kpi =direct=> [D*+:MC =direct=> [D0:MC =direct=> K-:primaryMC pi+:primaryMC ] pi+:primaryMC] pi-:primaryMC',
            '',
            path=main)

        ntupler = register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('treeName', 'ntuple')
        ntupler.param('variables', ['isSignal'])
        ntupler.param('particleList', 'B0:DstD0Kpi')
        main.add_module(ntupler)

        b2test_utils.safe_process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        print(f"True candidates {allSig}")
        print(f"False candidates {allBkg}")

        sig_expected = 406

        self.assertTrue(
            allSig == sig_expected,
            f"n_sig expected: {sig_expected} found: {sig_expected}.")
        self.assertTrue(allBkg == 0, f"n_bkg expected 0, found: {allBkg}.")

        print("Test passed, cleaning up.")

    def testReconstructMCDecay_3KS(self):
        """Reconstruct/search for an MC decay chain using the reconstructMCDecay tool."""

        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        inputfile = b2test_utils.require_file('Bdto3Ks_MC13aBG1_train.root', 'examples', py_case=self)
        inputMdst(inputfile, path=main)

        fillParticleListFromMC('pi+:primaryMC', 'mcPrimary', path=main)
        reconstructMCDecay('K_S0:MC =direct=> pi+:primaryMC pi-:primaryMC', '', path=main)
        reconstructMCDecay('B0:3KS =direct=> K_S0:MC K_S0:MC K_S0:MC', '', path=main)

        ntuplerKS = register_module('VariablesToNtuple')
        ntuplerKS.param('fileName', testFile.name)
        ntuplerKS.param('treeName', 'ntuple')
        ntuplerKS.param('variables', ['isSignal'])
        ntuplerKS.param('particleList', 'B0:3KS')
        main.add_module(ntuplerKS)

        b2test_utils.safe_process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        print(f"True candidates {allSig}")
        print(f"False candidates {allBkg}")

        sig_expected = 10

        self.assertTrue(
            allSig == sig_expected,
            f"n_sig expected: {sig_expected} found: {sig_expected}.")
        self.assertTrue(allBkg == 0, f"n_bkg expected 0, found: {allBkg}.")

        print("Test passed, cleaning up.")

    def testMCDecayFinder(self):
        """Reconstruct/search for an MC decay chain using the findMCDecay tool."""

        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        inputfile = b2test_utils.require_file(
            'analysis/1000_B_DstD0Kpi_skimmed.root', 'validation', py_case=self)
        inputMdst(inputfile, path=main)

        findMCDecay(
            'B0:DstD0Kpi',
            'B0 =direct=> [D*+ =direct=> [D0 =direct=> K- pi+ ] pi+] pi-',
            path=main)

        ntupler = register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['isSignal'])
        ntupler.param('particleList', 'B0:DstD0Kpi')
        main.add_module(ntupler)

        b2test_utils.safe_process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        print(f"True candidates {allSig}")
        print(f"False candidates {allBkg}")

        sig_expected = 406

        self.assertTrue(
            allSig == sig_expected,
            f"n_sig expected: {sig_expected} found: {sig_expected}.")
        self.assertTrue(allBkg == 0, f"n_bkg expected 0, found: {allBkg}.")

        print("Test passed, cleaning up.")

    def testMCDecayFinder_3KS(self):
        """Reconstruct/search for an MC decay chain using the findMCDecay tool."""

        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        inputfile = b2test_utils.require_file('Bdto3Ks_MC13aBG1_train.root', 'examples', py_case=self)
        inputMdst(inputfile, path=main)

        findMCDecay(
            'B0:3KS',
            'B0 -> [K_S0 -> pi+ pi-] [K_S0 -> pi+ pi-] [K_S0 -> pi+ pi-]',
            path=main)

        ntupler = register_module('VariablesToNtuple')
        ntupler.param('fileName', testFile.name)
        ntupler.param('variables', ['isSignal'])
        ntupler.param('particleList', 'B0:3KS')
        main.add_module(ntupler)

        b2test_utils.safe_process(main)

        ntuplefile = TFile(testFile.name)
        ntuple = ntuplefile.Get('ntuple')

        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty.")

        allBkg = ntuple.GetEntries("isSignal == 0")
        allSig = ntuple.GetEntries("isSignal > 0")

        print(f"True candidates {allSig}")
        print(f"False candidates {allBkg}")

        sig_expected = 10

        self.assertTrue(
            allSig == sig_expected,
            f"n_sig expected: {sig_expected} found: {sig_expected}.")
        self.assertTrue(allBkg == 0, f"n_bkg expected 0, found: {allBkg}.")

        print("Test passed, cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
