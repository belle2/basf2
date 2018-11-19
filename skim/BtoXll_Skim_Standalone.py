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
from skim.standardparticles.lightmesons import *
from stdPhotons import *
set_log_level(LogLevel.INFO)
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
import sys
import os
import glob
skimCode = encodeSkimName('BtoXll')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList)
loadStdSkimPi0()
loadStdSkimPhoton()
stdPi0s('loose')
stdPhotons('loose')
loadStdCharged()
stdK('95eff')
stdPi('95eff')
stdE('95eff')
stdMu('95eff')
stdMu('90eff')
stdKshorts()
loadStdLightMesons()

# EWP Skim
from BtoXll_List import *
XllList = B2XllList()
skimOutputUdst(skimCode, XllList)
summaryOfLists(XllList)


setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
