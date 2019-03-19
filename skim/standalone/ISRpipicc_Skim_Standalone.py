#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmonium skims
# Sen Jia, 27/Sep/2018
# ISRpipicc_List skim
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-03'
set_log_level(LogLevel.INFO)

import os
import sys
import glob

# create a new path
ISRskimpath = Path()

# Add default samples
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=ISRskimpath)

# use standard final state particle lists
stdPi('loose', path=ISRskimpath)
stdK('loose', path=ISRskimpath)
stdE('loose', path=ISRskimpath)
stdMu('loose', path=ISRskimpath)
stdPi('all', path=ISRskimpath)
stdK('all', path=ISRskimpath)
stdE('all', path=ISRskimpath)
stdMu('all', path=ISRskimpath)

# importing the reconstructed events from the ISRpipicc_List file
from skim.quarkonium import ISRpipiccList
ISRpipicc = ISRpipiccList(path=ISRskimpath)

# output to Udst file
skimCode = encodeSkimName('ISRpipicc')
skimOutputUdst(skimCode, ISRpipicc, path=ISRskimpath)

# print out Particle List statistics
summaryOfLists(ISRpipicc, path=ISRskimpath)

# output skim log information
setSkimLogging(path=ISRskimpath)

# process the path
process(ISRskimpath)

# print out the summary
print(statistics)
