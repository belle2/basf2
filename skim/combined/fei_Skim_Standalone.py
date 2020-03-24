#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI Hadronic and SL B0 and B+ tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180100, 11180200, 11180300, 11180400
    fei training: MC11 based, release-04-00-00 'FEIv4_2018_MC11_release_03_00_00'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]

######################################################
#
# This script reconstructs hadronic Btags using
# generically trained FEI.
#
# FEIv4_2018_MC11_release_03_00_00
#
#####################################################


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert


fileList = expert.get_test_file("MC12_mixedBGx1")
path = b2.create_path()
ma.inputMdstList('default', fileList, path=path)

from skim.fei import B0Hadronic, BplusHadronic, B0SL, BplusSL, runFEIforSkimCombined
# run pre-selection cuts and FEI
runFEIforSkimCombined(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B0 tag cuts
B0HadronicList = B0Hadronic(path)
skimCode1 = expert.encodeSkimName('feiHadronicB0')
expert.skimOutputUdst(skimCode1, B0HadronicList, path=path)
ma.summaryOfLists(B0HadronicList, path=path)

B0semileptonicList = B0SL(path)
skimCode2 = expert.encodeSkimName('feiSLB0')
expert.skimOutputUdst(skimCode2, B0semileptonicList, path=path)
ma.summaryOfLists(B0semileptonicList, path=path)

# Apply final B+ tag cuts
BphadronicList = BplusHadronic(path)
skimCode3 = expert.encodeSkimName('feiHadronicBplus')
expert.skimOutputUdst(skimCode3, BphadronicList, path=path)
ma.summaryOfLists(BphadronicList, path=path)

BpsemileptonicList = BplusSL(path)
skimCode4 = expert.encodeSkimName('feiSLBplus')
expert.skimOutputUdst(skimCode4, BpsemileptonicList, path=path)
ma.summaryOfLists(BpsemileptonicList, path=path)

expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
