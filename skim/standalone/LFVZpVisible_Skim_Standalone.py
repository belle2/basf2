#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Skims for LFV Z'
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'
skimCode = encodeSkimName('LFVZpVisible')
import sys
import os
import glob

lfvzppath = Path()
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=lfvzppath)

stdPi('loose', path=lfvzppath)
stdK('loose', path=lfvzppath)
stdE('loose', path=lfvzppath)
stdMu('loose', path=lfvzppath)
stdPi('all', path=lfvzppath)
stdK('all', path=lfvzppath)
stdE('all', path=lfvzppath)
stdMu('all', path=lfvzppath)


from skim.dark import LFVZpVisibleList
SysList = LFVZpVisibleList(path=lfvzppath)
skimOutputUdst(skimCode, SysList, path=lfvzppath)
summaryOfLists(SysList, path=lfvzppath)

setSkimLogging(path=lfvzppath)
process(lfvzppath)

print(statistics)
