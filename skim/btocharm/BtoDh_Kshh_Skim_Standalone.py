#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(*) h(*) skims
# P. Urquijo, 6/Jan/2015
#
######################################################
from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdV0s import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'

import os.path
import sys
import glob
import basf2

path = basf2.Path()

skimCode = encodeSkimName('BtoDh_Kshh')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path)


stdPi('all', path)
stdK('all', path)
stdKshorts(path)

# B- to D(->Kshh)h- Skim
from skim.btocharm import loadDkshh, BsigToDhToKshhList
loadDkshh(path)
BtoDhList = BsigToDhToKshhList(path)
skimOutputUdst(skimCode, BtoDhList)
summaryOfLists(BtoDhList)


setSkimLogging(path)
process(path)

# print out the summary
print(statistics, path)
