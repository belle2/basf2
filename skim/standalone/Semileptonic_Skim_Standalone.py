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
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from skim.standardparticles.charm import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'
set_log_level(LogLevel.INFO)
import sys
import os
import glob


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)
stdPi0s('loose')
stdPhotons('loose')
loadStdCharged()
loadStdSkimPi0()
loadStdSkimPhoton()
stdKshorts()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# SL Skim
from skim.semileptonic import SemileptonicList
SLList = SemileptonicList()
skimCode1 = encodeSkimName('SLUntagged')
print(skimCode1)
skimOutputUdst(skimCode1, SLList)
summaryOfLists(SLList)


from skim.leptonic import LeptonicList
lepList = LeptonicList()
skimCode2 = encodeSkimName('LeptonicUntagged')
print(skimCode2)
skimOutputUdst(skimCode2, lepList)
summaryOfLists(lepList)


from skim.semileptonic import PRList
PRList = PRList()
skimCode3 = encodeSkimName('PRsemileptonicUntagged')
skimOutputUdst(skimCode3, PRList)


summaryOfLists(PRList)
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
