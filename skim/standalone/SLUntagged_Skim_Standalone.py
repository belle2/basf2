#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
# P. Urquijo, 6/Jan/2015
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
b2.set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = expert.encodeSkimName('SLUntagged')

SLpath = b2.Path()
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=SLpath)

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
expert.skimOutputUdst(skimCode, SLList, path=SLpath)

ma.summaryOfLists(SLList, path=SLpath)


expert.setSkimLogging(path=SLpath)
b2.process(SLpath)

# print out the summary
print(statistics)
