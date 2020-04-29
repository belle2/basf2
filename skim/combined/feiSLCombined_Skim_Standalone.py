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


fileList = expert.get_test_file("MC12_mixedBGx1")
path = b2.create_path()
ma.inputMdstList('default', fileList, path=path)

from skim.fei import B0SL, BplusSL, runFEIforSLCombined
# run pre-selection  cuts and FEI
path2 = runFEIforSLCombined(path)

# Include MC matching
path2.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)
path2.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B0 semileptonic tag cuts
B0semileptonicList = B0SL(path2)
skimCode1 = expert.encodeSkimName('feiSLB0')
expert.skimOutputUdst(skimCode1, B0semileptonicList, path=path2)
ma.summaryOfLists(B0semileptonicList, path=path2)

# Apply final B+ semileptonic tag cuts
BpsemileptonicList = BplusSL(path2)
skimCode2 = expert.encodeSkimName('feiSLBplus')
expert.skimOutputUdst(skimCode2, BpsemileptonicList, path=path2)
ma.summaryOfLists(BpsemileptonicList, path=path2)


expert.setSkimLogging(path2)
b2.process(path)

# print out the summary
print(b2.statistics)
