#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematics Skims for radiative electron pairs
# Sam Cunliffe (sam.cunliffe@desy.de), 2018
#
#######################################################

from basf2 import process, statistics, Path
from modularAnalysis import inputMdstList, skimOutputUdst, summaryOfLists
from stdCharged import stdE
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'

# create a path to build skim lists
skimpath = Path()

# some test input data
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=skimpath)
stdE('all', path=skimpath)

# setup the skim get the skim code
from skim.systematics import SystematicsRadEEList
radeelist = SystematicsRadEEList(path=skimpath)
skimcode = encodeSkimName('SystematicsRadEE')
skimOutputUdst(skimcode, radeelist, path=skimpath)
summaryOfLists(radeelist, path=skimpath)

# silence noisy modules
setSkimLogging(path=skimpath)

# process the path (run the skim)
process(skimpath)
print(statistics)
