#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematics Skims for radiative electron pairs
# Sam Cunliffe (sam.cunliffe@desy.de), 2018
#
#######################################################

import basf2 as b2
from modularAnalysis import inputMdstList, \
    skimOutputUdst, summaryOfLists
from stdCharged import stdE
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

# create a path to build skim lists
skimpath = b2.Path()

# some test input data
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=skimpath)
stdE('all', path=skimpath)

# setup the skim get the skim code
from skim.systematics import SystematicsRadEEList
radeelist = SystematicsRadEEList(path=skimpath)
skimcode = expert.encodeSkimName('SystematicsRadEE')
expert.skimOutputUdst(skimcode, radeelist, path=skimpath)
ma.summaryOfLists(radeelist, path=skimpath)

# silence noisy modules
expert.setSkimLogging(path=skimpath)

# process the path (run the skim)
b2.process(skimpath)
print(statistics)
