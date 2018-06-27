#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('SLUntagged')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList)

loadStdCharged()

stdPi0s('loose')  # for stdCharm.py
stdPhotons('loose')
loadStdKS()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# SL Skim
from SLUntagged_List import *
SLList = SemileptonicList()
skimOutputUdst(skimCode, SLList)

summaryOfLists(SLList)


setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
