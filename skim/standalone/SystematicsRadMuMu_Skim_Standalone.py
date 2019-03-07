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
from stdCharged import stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'

# create a path to build skim lists
skimpath = Path()

# input test data
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=skimpath)
stdMu('all', path=skimpath)

# setup the skim get the skim code
from skim.systematics import SystematicsRadMuMuList
SysList = SystematicsRadMuMuList(skimpath)
skimCode = encodeSkimName('SystematicsRadMuMu')
skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

# silence noisy modules
setSkimLogging(path=skimpath)

# process the path (run the skim)
process(skimpath)
print(statistics)
