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
set_log_level(LogLevel.INFO)
import sys
import os
import glob
gb2_setuprel = 'release-01-00-00'


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)

loadStdSkimPi0()
loadStdSkimPhoton()
loadStdCharged()
stdPi0s('loose')  # for stdCharm.py
stdPhotons('loose')
loadStdKS()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# SL Skim
from LeptonicUntagged_List import *

lepList = LeptonicList()
skimOutputUdst('LeptonicUntagged', lepList)

summaryOfLists(lepList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
