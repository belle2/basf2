#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for four-lepton events
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging


set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'

skimCode = encodeSkimName('SystematicsEELL')
import sys
import os
import glob

skimpath = Path()


fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=skimpath)

stdE('all', path=skimpath)
stdMu('all', path=skimpath)

from skim.systematics import *
SysList = EELLList(skimpath)
skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)
