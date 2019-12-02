#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
    FEI Hadronic B+ tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180200
    fei training: MC12 based, release-04-00-00 'FEIv4_2019_MC12_release_03_01_01'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('feiHadronicBplus')
fileList = expert.get_test_file("mixedBGx1", "MC12")

path = create_path()

ma.inputMdstList('default', fileList, path=path)

from skim.fei import *
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
