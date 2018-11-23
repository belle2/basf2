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
from stdCharged import stdPi, stdK
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'
import os
import sys
import glob
from skimExpertFunctions import *
skimCode = encodeSkimName('BtoDh_hh')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList)


# create and fill pion and kaon ParticleLists
stdPi('all')
stdK('all')

# B+ to D(->h+h-)h+ Skim
from skim.btocharm import loadD0bar, BsigToDhTohhList
loadD0bar()
BtoDhList = BsigToDhTohhList()
skimOutputUdst(skimCode, BtoDhList)
summaryOfLists(BtoDhList)


setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
