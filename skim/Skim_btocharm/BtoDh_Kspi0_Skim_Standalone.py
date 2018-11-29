#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kspi0) h skims
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
gb2_setuprel = 'release-02-00-01'

import os.path
import sys
import glob

kspi0path = Path()

skimCode = encodeSkimName('BtoDh_Kspi0')
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList, path=kspi0path)


stdPi('all', path=kspi0path)
stdK('all', path=kspi0path)
loadStdSkimPi0(path=kspi0path)
stdKshorts(path=kspi0path)

# B- to D(->Kspi0)h- Skim
from skim.btocharm import loadDkspi0, BsigToDhToKspi0List
loadDkspi0(path=kspi0path)
BtoDhList = BsigToDhToKspi0List(path=kspi0path)
skimOutputUdst(skimCode, BtoDhList, path=kspi0path)
summaryOfLists(BtoDhList, path=kspi0path)


setSkimLogging()
process(kspi0path)

# print out the summary
print(statistics, path=kspi0path)
