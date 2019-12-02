#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for radiative muon pairs
# Torben Ferber (torben.ferber@desy.de), 2018
# Sam Cunliffe (sam.cunliffe@desy.de)
#
#######################################################

import basf2 as b2
from stdCharged import stdMu
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)
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
print(b2.statistics)
