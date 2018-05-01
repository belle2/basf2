#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# HIGH RETENTION RATE NOT PRODUCED
#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-00'
import sys
import os
import glob
scriptName = sys.argv[0]
skimListName = scriptName[:-19]
skimCode = encodeSkimName(skimListName)
print(skimListName)
print(skimCode)
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)


loadStdCharged()

from SystematicsLambda_List import *
SysList = SystematicsList()
skimOutputUdst('SystematicsLambda', SysList)
summaryOfLists(SysList)

process(analysis_main)

print(statistics)
