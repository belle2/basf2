#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
    FEI Hadronic B+ tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180200
    fei training: MC13 based, release-04-01-01 'feiv4_14012020_MC13_release_04_01_01_phase3'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('feiHadronicBplus')
fileList = expert.get_test_file("MC12_mixedBGx1")

path = b2.create_path()

ma.inputMdstList('default', fileList, path=path)

from skim.fei import BplusHadronic, runFEIforBplusHadronic
# run pre-selection cuts and FEI
runFEIforBplusHadronic(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)

# Apply final B+ tag cuts
BplusHadronicList = BplusHadronic(path)
expert.skimOutputUdst(skimCode, BplusHadronicList, path=path)
ma.summaryOfLists(BplusHadronicList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
