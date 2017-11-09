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
gb2_setuprel = 'build-2017-10-16'
set_log_level(LogLevel.INFO)
import sys
import os
import glob

inputMdstList('default', fileList)
stdPi0s('loose')
stdPhotons('loose')
loadStdCharged()
loadStdSkimPi0()
loadStdSkimPhoton()
loadStdKS()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# SL Skim
from Semileptonic_List import *
SLList = SemileptonicList()
skimOutputUdst('SLUntagged', SLList)
summaryOfLists(SLList)

lepList = LeptonicList()
skimOutputUdst('LeptonicUntagged', lepList)
summaryOfLists(lepList)

process(analysis_main)

# print out the summary
print(statistics)
