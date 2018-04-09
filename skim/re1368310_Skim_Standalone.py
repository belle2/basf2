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
from stdCharm import *
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
stdPi0s('loose')
# stdPhotons('loose')
stdPhotons('tight')  # also builds loose list
loadStdSkimPhoton()
loadStdSkimPi0()
loadStdCharged()
stdK('95eff')
stdPi('95eff')
stdKshorts()
loadStdLightMesons()

# EWP Skim
from re1368310_List import *
XgammaList = B2XgammaList()
skimOutputUdst('re1368310', XgammaList)
summaryOfLists(XgammaList)


printDataStore()

process(analysis_main)

# print out the summary
print(statistics)
