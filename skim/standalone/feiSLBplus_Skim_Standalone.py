#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""FEI Semi-leptonic B0 tag skim standalone for generic analysis in th
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180400
    fei training: MC13 based, release-04-01-01 'FEIv4_2020_MC13_release_04_01_01'
 """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-01-01'
skimCode = expert.encodeSkimName('feiSLBplus')
fileList = expert.get_test_file("MC12_mixedBGx1")

path = b2.create_path()

ma.inputMdstList('default', fileList, path=path)

from skim.fei import BplusSL, runFEIforBplusSL
# run pre-selection cuts and FEI
runFEIforBplusSL(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B+ tag cuts
BtagList = BplusSL(path)
expert.skimOutputUdst(skimCode, BtagList, path=path)
ma.summaryOfLists(BtagList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path, ['ParticleCombiner'])
b2.process(path)

# print out the summary
print(b2.statistics)
