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
gb2_setuprel = 'release-03-00-03'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('SLUntagged')

SLpath = Path()
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=SLpath)

stdPi('loose', path=SLpath)
stdK('loose', path=SLpath)
stdPi('all', path=SLpath)
stdE('all', path=SLpath)
stdMu('all', path=SLpath)

stdPi0s('loose', path=SLpath)  # for skim.standardlists.charm
stdPhotons('loose', path=SLpath)
stdKshorts(path=SLpath)

loadStdD0(path=SLpath)
loadStdDplus(path=SLpath)
loadStdDstar0(path=SLpath)
loadStdDstarPlus(path=SLpath)

# SL Skim
from skim.semileptonic import SemileptonicList
SLList = SemileptonicList(SLpath)
skimOutputUdst(skimCode, SLList, path=SLpath)

summaryOfLists(SLList, path=SLpath)


setSkimLogging(path=SLpath)
process(SLpath)

# print out the summary
print(statistics)
