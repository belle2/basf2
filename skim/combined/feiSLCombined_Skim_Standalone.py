#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""FEI Semi-leptonic B0 and B+ tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180300, 11180400
    fei training: MC11 based, release-04-00-00 'FEIv4_2018_MC11_release_03_00_00'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

fileList = expert.get_test_file("mixedBGx1", "MC12")
path = create_path()
ma.inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection  cuts and FEI
runFEIforSLCombined(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B0 semileptonic tag cuts
B0semileptonicList = B0SL(path)
skimCode1 = expert.encodeSkimName('feiSLB0')
expert.skimOutputUdst(skimCode1, B0semileptonicList, path=path)
ma.summaryOfLists(B0semileptonicList, path=path)

# Apply final B+ semileptonic tag cuts
BpsemileptonicList = BplusSL(path)
skimCode2 = expert.encodeSkimName('feiSLBplus')
expert.skimOutputUdst(skimCode2, BpsemileptonicList, path=path)
ma.summaryOfLists(BpsemileptonicList, path=path)


expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(statistics)
