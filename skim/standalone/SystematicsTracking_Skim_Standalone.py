#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematic Tracking
# Y. Kato, Apr/2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPhotons import *
from stdPi0s import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
set_log_level(LogLevel.INFO)
gb_setuprel = 'release-03-02-00'

import sys
import os
import glob

argvs = sys.argv
argc = len(argvs)

skimpath = Path()
fileList = get_test_file("mixedBGx1", "MC12")


inputMdstList('default', fileList, path=skimpath)
stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdPi0s('loose', path=skimpath)

skimCode = encodeSkimName('SystematicsTracking')

from skim.systematics import SystematicsTrackingList
SysList = SystematicsTrackingList(skimpath)

skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)
