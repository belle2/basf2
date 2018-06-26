#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmonium skims
# Sen Jia, 21/Mar/2018
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

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('default', fileList)

loadStdCharged()
stdPhotons('tight')

from ISRpipicc_List import *
ISRpipicc = ISRpipiccList()
skimOutputUdst(skimCode, ISRpipicc)
summaryOfLists(ISRpipicc)


setSkimLogging()
process(analysis_main)

print(statistics)
