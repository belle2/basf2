#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Double charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *

set_log_level(LogLevel.INFO)


gb2_setuprel = 'release-01-00-00'
import sys
import os
import glob

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)
loadStdCharged()
loadStdKS()
loadStdSkimPi0()
loadStdSkimPhoton()
stdPi0s('loose')
stdPhotons('loose')
loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# Double Charm Skim
from DoubleCharm_List import *
DCList = DoubleCharmList()
skimOutputUdst('DoubleCharm', DCList)
summaryOfLists(DCList)
process(analysis_main)

# print out the summary
print(statistics)
