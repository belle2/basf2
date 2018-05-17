#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Hulya Atmacan 2016/Oct/25
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-00'
import os
import sys
import glob
from skimExpertFunctions import *
scriptName = sys.argv[0]
skimListName = scriptName[:-19]
skimCode = encodeSkimName(skimListName)
print(skimListName)
print(skimCode)

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)


# create and fill pion and kaon ParticleLists
# second argument are the selection criteria: '' means no cut, take all
loadStdCharged()

# B+ to D(->h+h-)h+ Skim
from BtoDh_hh_List import *
loadD0bar()
BtoDhList = BsigToDhTohhList()
skimOutputUdst(skimListName, BtoDhList)
summaryOfLists(BtoDhList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)


for module in analysis_main.modules():
    if module.type() == "ParticleVertexFitter":
        module.set_log_level(LogLevel.ERROR)


process(analysis_main)

# print out the summary
print(statistics)
