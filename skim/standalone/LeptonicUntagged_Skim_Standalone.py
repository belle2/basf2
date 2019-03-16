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
from stdV0s import stdKshorts
from skim.standardlists.charm import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
import sys
import os
import glob
gb2_setuprel = 'release-03-00-03'
skimCode = encodeSkimName('LeptonicUntagged')

fileList = get_test_file("mixedBGx1", "MC11")

leppath = Path()

inputMdstList('default', fileList, path=leppath)

loadStdSkimPi0(path=leppath)
loadStdSkimPhoton(path=leppath)
stdPi('loose', path=leppath)
stdK('loose', path=leppath)
stdPi('all', path=leppath)
stdE('all', path=leppath)
stdMu('all', path=leppath)
stdPi0s('loose', path=leppath)  # for stdCharm.py
stdPhotons('loose', path=leppath)
stdKshorts(path=leppath)

loadStdD0(path=leppath)
loadStdDplus(path=leppath)
loadStdDstar0(path=leppath)
loadStdDstarPlus(path=leppath)

# SL Skim
from skim.leptonic import LeptonicList

lepList = LeptonicList(path=leppath)
skimOutputUdst(skimCode, lepList, path=leppath)

summaryOfLists(lepList, path=leppath)


setSkimLogging(path=leppath)
process(leppath)

# print out the summary
print(statistics)
