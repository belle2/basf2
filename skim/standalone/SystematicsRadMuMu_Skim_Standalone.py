#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for radiative muon pairs
# Torben Ferber (torben.ferber@desy.de), 2018
# Sam Cunliffe (sam.cunliffe@desy.de)
#
#######################################################

from basf2 import process, statistics, Path, set_log_level, LogLevel
from modularAnalysis import inputMdstList, skimOutputUdst, summaryOfLists
from stdCharged import stdE, stdK, stdMu, stdPi
import skimExpertFunctions as expert
b2.set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

# create a path to build skim lists
skimpath = b2.Path()

# input test data
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=skimpath)
stdMu('all', path=skimpath)

# setup the skim get the skim code
from skim.systematics import SystematicsRadMuMuList
SysList = SystematicsRadMuMuList(skimpath)
skimCode = expert.encodeSkimName('SystematicsRadMuMu')
expert.skimOutputUdst(skimCode, SysList, path=skimpath)
ma.summaryOfLists(SysList, path=skimpath)

# silence noisy modules
expert.setSkimLogging(path=skimpath)

# process the path (run the skim)
b2.process(skimpath)
print(statistics)
