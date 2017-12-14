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


fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]
inputMdstList('default', fileList)

stdPhotons('loose')
stdPi0s('loose')
loadStdCharged()
loadStdSkimPi0()  # for stdCharm.py
loadStdSkimPhoton()
loadStdKS()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# SL Skim
from SLUntagged_List import *
SLList = SemileptonicList()
skimOutputUdst('SLUntagged', SLList)
summaryOfLists(SLList)


process(analysis_main)

# print out the summary
print(statistics)
