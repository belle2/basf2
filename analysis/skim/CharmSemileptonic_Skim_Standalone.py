#!/usr/bin/evn python3
# -*- coding: utf-8 -*-

##################################################
#
# Charm SL skims
# J. Bennett, 5/Oct/2016
#
##################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *

set_log_level(LogLevel.INFO)

filelist = [
    '/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000018/s00/e0001/4S/r00001/ccbar/sub00/' +
    'mdst_000001_prod00000018_task00000001.root'
]

inputMdstList('MC5', filelist)

loadStdCharged()
loadStdPi0()
loadStdKS()

loadStdD0()
loadStdDplus()

# CSL Skim
from CharmSemileptonic_List import *
CSLList = CharmSemileptonicList()
skimOutputUdst('CharmSL_Standalone', CSLList)
summaryOfLists(CSLList)

process(analysis_main)

# print out the summary
print(statistics)
