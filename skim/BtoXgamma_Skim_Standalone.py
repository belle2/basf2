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

fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]


inputMdstList('default', fileList)
stdPi0s('loose')
stdPhotons('loose')
loadStdSkimPhoton()
loadStdSkimPi0()
loadStdCharged()
stdKshorts()
loadStdLightMesons()
cutAndCopyList('gamma:E15', 'gamma:skim', '1.5 < E < 100')

# EWP Skim
from BtoXgamma_List import *
XgammaList = B2XgammaList()
skimOutputUdst('BtoXgamma', XgammaList)
summaryOfLists(XgammaList)


# printDataStore()

process(analysis_main)

# print out the summary
print(statistics)
