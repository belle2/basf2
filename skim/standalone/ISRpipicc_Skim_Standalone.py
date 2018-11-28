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
gb2_setuprel = 'release-02-00-01'
set_log_level(LogLevel.INFO)

import os
import sys
import glob

# create a new path
ISRskimpath = Path()

# Add MC9 samples
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('MC9', fileList, path=ISRskimpath)

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
setSkimLogging(skim_path=ISRskimpath)

# process the path
process(ISRskimpath)

# print out the summary
print(statistics)
