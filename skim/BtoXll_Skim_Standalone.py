#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP standalone skim steering
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdLightMesons import *
from stdPhotons import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'build-2017-10-16'
import sys
import os
import glob


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)
loadStdSkimPi0()
loadStdSkimPhoton()
stdPi0s('loose')
stdPhotons('loose')
loadStdCharged()
stdKshorts()
loadStdLightMesons()
cutAndCopyList('gamma:E15', 'gamma:skim', '1.5<E<100')
# EWP Skim
from BtoXll_List import *

XllList = B2XllList()
skimOutputUdst('BtoXll', XllList)
summaryOfLists(XllList)

# printDataStore()

process(analysis_main)

# print out the summary
print(statistics)
