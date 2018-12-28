#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-00'
set_log_level(LogLevel.INFO)
import sys
import os
import glob


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

SLskimpath = Path()

inputMdstList('MC9', fileList, path=SLskimpath)
stdPi0s('loose', path=SLskimpath)
stdPhotons('loose', path=SLskimpath)
stdPi('loose', path=SLskimpath)
stdK('loose', path=SLskimpath)
stdPi('all', path=SLskimpath)
stdE('all', path=SLskimpath)
stdMu('all', path=SLskimpath)
loadStdSkimPi0(path=SLskimpath)
loadStdSkimPhoton(path=SLskimpath)
stdKshorts(path=SLskimpath)

loadStdD0(path=SLskimpath)
loadStdDplus(path=SLskimpath)
loadStdDstar0(path=SLskimpath)
loadStdDstarPlus(path=SLskimpath)

# SL Skim
from skim.semileptonic import SemileptonicList
SLList = SemileptonicList(SLskimpath)
skimCode1 = encodeSkimName('SLUntagged')
print(skimCode1)
skimOutputUdst(skimCode1, SLList, path=SLskimpath)
summaryOfLists(SLList, path=SLskimpath)


from skim.leptonic import LeptonicList
lepList = LeptonicList(SLskimpath)
skimCode2 = encodeSkimName('LeptonicUntagged')
print(skimCode2)
skimOutputUdst(skimCode2, lepList, path=SLskimpath)
summaryOfLists(lepList, path=SLskimpath)


from skim.semileptonic import PRList
PRList = PRList(path=SLskimpath)
skimCode3 = encodeSkimName('PRsemileptonicUntagged')
skimOutputUdst(skimCode3, PRList, path=SLskimpath)


summaryOfLists(PRList, path=SLskimpath)
setSkimLogging(path=SLskimpath)
process(SLskimpath)

# print out the summary
print(statistics)
