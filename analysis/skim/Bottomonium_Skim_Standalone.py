#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro, 25/Jul/2016
#
######################################################

from basf2 import *
from modularAnalysis import *

set_log_level(LogLevel.ERROR)

filelist = \
    filelist = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000033/s00/e0000/4S/r00000/bhabha/sub00/' +
     'mdst_00000*.root'
     ]

inputMdstList('default', filelist)

# Bottomonium Skim
from Bottomonium_List import *
YList = UpsilonList()
skimOutputUdst('Y_Standalone_bhabha', YList)
summaryOfLists(YList)
process(analysis_main)

# print out the summary
print(statistics)
