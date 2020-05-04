#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematic Tracking
# Y. Kato, Apr/2018
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPi0s import stdPi0s
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

import sys

argvs = sys.argv
argc = len(argvs)

skimpath = b2.Path()
fileList = expert.get_test_file("MC12_mixedBGx1")


ma.inputMdstList('default', fileList, path=skimpath)
stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdPi0s('eff40_Jan2020', path=skimpath)

skimCode = expert.encodeSkimName('SystematicsTracking')

from skim.systematics import SystematicsTrackingList
SysList = SystematicsTrackingList(skimpath)

expert.skimOutputUdst(skimCode, SysList, path=skimpath)
ma.summaryOfLists(SysList, path=skimpath)

expert.setSkimLogging(path=skimpath)
b2.process(skimpath)

print(b2.statistics)
