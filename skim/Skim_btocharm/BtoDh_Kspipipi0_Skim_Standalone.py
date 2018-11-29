#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kspipipi0) h skims
# Minakshi Nayak, 24/Jan/2017
#
######################################################
from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'

import os.path
import sys
import glob

kspipipi0path = Path()

skimCode = encodeSkimName('BtoDh_Kspipipi0')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList, path=kspipipi0path)


stdPi('all', path=kspipipi0path)
stdK('all', path=kspipipi0path)
stdKshorts(path=kspipipi0path)
loadStdSkimPi0(path=kspipipi0path)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import loadDkspipipi0, BsigToDhToKspipipi0List
loadDkspipipi0(path=kspipipi0path)
BtoDhList = BsigToDhToKspipipi0List(path=kspipipi0path)
skimOutputUdst(skimCode, BtoDhList, path=kspipipi0path)
summaryOfLists(BtoDhList, path=kspipipi0path)


setSkimLogging()
process(kspipipi0path)

# print out the summary
print(statistics, path=kspipipi0path)
