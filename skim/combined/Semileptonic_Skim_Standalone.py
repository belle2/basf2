#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-04-00-00'
set_log_level(LogLevel.INFO)
import sys
import os
import glob
fileList = get_test_file("mixedBGx1", "MC12")
SLskimpath = Path()

inputMdstList('default', fileList, path=SLskimpath)
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
