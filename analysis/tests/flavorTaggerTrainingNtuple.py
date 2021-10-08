#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


"""
This file tests the functionality of sampling needed to train the flavor tagger.
"""

import b2test_utils
import basf2
from basf2 import set_random_seed, create_path, process
import modularAnalysis as ma
from variables import variables as vm
import flavorTagger as ft
import ROOT
import os
import math

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed("1337")
testinput = [b2test_utils.require_file('analysis/tests/Btonunubar.root')]

###############################################################################
# Create signal B0 particle and produce pseudo training ntuples
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=testinput)

ma.fillParticleListFromMC('nu_tau', '', path=testpath)
ma.reconstructMCDecay(decayString='B0:sig -> nu_tau anti-nu_tau', cut='', path=testpath)

# Test to build a rest of event from the MC B0 decaying to two neutrinos
ma.buildRestOfEvent('B0:sig', path=testpath)

# Test MC association of MC particle
ma.applyCuts('B0:sig', ' abs(isRelatedRestOfEventB0Flavor) == 1', path=testpath)

###############################################################################
# Test functionality of sampling
roe_path = create_path()
deadEndPath = create_path()

ma.signalSideParticleListsFilter(
            ['B0:sig'],
            'nROE_Charged(, 0) > 0 and abs(qrCombined) == 1',
            roe_path,
            deadEndPath)

# Create list of ROE electrons
ma.fillParticleList('e+:inRoe', 'isInRestOfEvent > 0.5 and passesROEMask() > 0.5 and ' +
                    'isNAN(p) !=1 and isInfinity(p) != 1', path=roe_path)

# Save pseudo training sample
methodPrefixEventLevel = "FlavorTagger_Belle2_B2nunuBGx1EventLevelElectronFBDT"
targetVariable = 'isRightCategory(Electron)'
ma.applyCuts('e+:inRoe', 'isRightCategory(mcAssociated) > 0', path=roe_path)

# Skip electrons if list is empty
eventLevelpath = create_path()
SkipEmptyParticleList = basf2.register_module("SkimFilter")
SkipEmptyParticleList.set_name('SkimFilter_EventLevelElectron')
SkipEmptyParticleList.param('particleLists', 'e+:inRoe')
SkipEmptyParticleList.if_true(eventLevelpath, basf2.AfterConditionPath.CONTINUE)
roe_path.add_module(SkipEmptyParticleList)

ntuple = basf2.register_module('VariablesToNtuple')
ntuple.param('fileName', methodPrefixEventLevel + "sampled0.root")
ntuple.param('treeName', methodPrefixEventLevel + "_tree")

# Call variable aliases from flavor tagger
ft.set_FlavorTagger_pid_aliases()

variablesToBeSaved = ['useCMSFrame(p)',
                      'useCMSFrame(pt)',
                      'p',
                      'pt',
                      'cosTheta',
                      'electronID',
                      'eid_TOP',
                      'eid_ARICH',
                      'eid_ECL',
                      'BtagToWBosonVariables(recoilMassSqrd)',
                      'BtagToWBosonVariables(pMissCMS)',
                      'BtagToWBosonVariables(cosThetaMissCMS)',
                      'BtagToWBosonVariables(EW90)',
                      'cosTPTO',
                      'chiProb',
                      'hasHighestProbInCat(e+:inRoe, isRightTrack(Electron))',
                      targetVariable, 'ancestorHasWhichFlavor',
                      'isSignal', 'mcPDG', 'mcErrors', 'genMotherPDG',
                      'nMCMatches', 'B0mcErrors'
                      ]
ntuple.param('variables', variablesToBeSaved)
ntuple.param('particleList', 'e+:inRoe')
eventLevelpath.add_module(ntuple)

testpath.for_each('RestOfEvent', 'RestOfEvents', roe_path)

###############################################################################

with b2test_utils.clean_working_directory():
    process(testpath, 5)

    # Testing
    assert os.path.isfile(methodPrefixEventLevel + "sampled0.root"), methodPrefixEventLevel + "sampled0.root" + " wasn't created"
    f = ROOT.TFile(methodPrefixEventLevel + "sampled0.root")
    t1 = f.Get(methodPrefixEventLevel + "_tree")
    assert bool(t1), methodPrefixEventLevel + "_tree" + " isn't contained in file"
    assert t1.GetEntries() > 0, methodPrefixEventLevel + "_tree" + "contains zero entries"
    for iVariable in variablesToBeSaved:
        iROOTVariable = str(ROOT.Belle2.makeROOTCompatible(iVariable))
        assert t1.GetListOfBranches().Contains(iROOTVariable),  iROOTVariable +\
            " branch is missing from " + methodPrefixEventLevel + "_tree"

    assert t1.GetEntries() == 40, "40 entries should be saved in the test training ntuple, otherwise some problem happened."

    mcPDGCodes = [
        211.0,        211.0,        211.0,        211.0,        211.0,        321.0,        13.0,        11.0,
        211.0,        211.0,        211.0,        11.0,        211.0,        211.0,        211.0,        211.0,
        211.0,        11.0,        211.0,        211.0,        211.0,        211.0,        211.0,        211.0,        211.0,
        2212.0,        211.0,        211.0,        2212.0,        211.0,        13.0,        211.0,        211.0,
        211.0,        211.0,        211.0,        211.0,        321.0,        321.0,        211.0]

    for iEntry in range(t1.GetEntries()):
        t1.GetEntry(iEntry)
        assert abs(t1.useCMSFrame__bop__bc) > 0, " p* should be greater than 0"
        assert abs(t1.useCMSFrame__bopt__bc) > 0, " pt* should be greater than 0"
        assert abs(t1.p) > 0, " p should be greater than 0"
        assert abs(t1.pt) > 0, " pt should be greater than 0"
        assert abs(t1.cosTheta) > 0, " cosTheta should be greater than 0"
        assert abs(t1.electronID) > 0, " electronID should be greater than 0"
        assert abs(t1.eid_TOP) > 0, " eid_TOP should be greater than 0"
        assert abs(t1.eid_ARICH) > 0, " eid_ARICH should be greater than 0"
        assert abs(t1.eid_ECL) > 0, "eid_ECL should be greater than 0"
        assert abs(t1.BtagToWBosonVariables__borecoilMassSqrd__bc) > 0, " recoilMassSqrd should be greater than 0"
        assert abs(t1.BtagToWBosonVariables__bopMissCMS__bc) > 0, " pMissCMS should be greater than 0"
        assert abs(t1.BtagToWBosonVariables__bocosThetaMissCMS__bc) > 0, " cosThetaMissCMS should be greater than 0"
        assert abs(t1.BtagToWBosonVariables__boEW90__bc) > 0, " EW90 should be greater than 0"
        assert abs(t1.cosTPTO) > 0, " cosTPTO should be greater than 0"
        assert abs(t1.chiProb) > 0, " chiProb should be greater than 0"
        if math.isnan(
            t1.hasHighestProbInCat__boe__pl__clinRoe__cm__spisRightTrack__boElectron__bc__bc
               ):
            basf2.B2FATAL(" hasHighestProbInCat Electron should not be nan")
        if math.isnan(t1.isRightCategory__boElectron__bc):
            basf2.B2FATAL(" isRightCategory Electron should not be nan ")
        if math.isnan(t1.ancestorHasWhichFlavor):
            basf2.B2FATAL(" ancestorHasWhichFlavor should not be nan")
        if math.isnan(t1.isSignal):
            basf2.B2FATAL(" isSignal should not be nan")
        assert abs(t1.mcPDG) == mcPDGCodes[iEntry], " Some mismatch between PDG codes happened in entry " + iEntry
        if math.isnan(t1.mcErrors):
            basf2.B2FATAL(" mcErrors should not be equal to nan")
        assert abs(t1.genMotherPDG) > 0, " genMotherPDG should be greater than 0"
        assert abs(t1.nMCMatches) > 0, " nMCMatches should be greater than 0"
