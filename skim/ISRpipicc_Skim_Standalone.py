#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmonium skims
# Sen Jia, 11/Jun/2018
# ISRpipicc_List skim
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('ISRpipicc')

# Add MC9 samples
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('default', fileList)

# use standard final state particle lists
loadStdCharged()
stdPhotons('tight')

# importing the reconstructed events from the ISRpipicc_List file
from ISRpipicc_List import *
ISRpipicc = ISRpipiccList()

# output to Udst file
skimOutputUdst(skimCode, ISRpipicc)

# print out Particle List statistics
summaryOfLists(ISRpipicc)

# output skim log information
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
