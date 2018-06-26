#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
# VERY HIGH RETENTION RATE
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-00'
skimCode = encodeSkimName('SystematicsXi')
import sys
import os
import glob

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)


loadStdCharged()

from SystematicsXi_List import *
SysList = SystematicsList()
skimOutputUdst(skimCode, SysList)
summaryOfLists(SysList)

setSkimLogging()
process(analysis_main)

print(statistics)
