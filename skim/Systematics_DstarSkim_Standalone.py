#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *

set_log_level(LogLevel.INFO)

filelist = [
    '/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/' +
    'MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/mdst_000001_prod00000198_task00000001.root'
]

inputMdstList('MC5', filelist)

loadStdCharged()

from Systematics_List import *
SysList = PiKFromDstarList()
skimOutputUdst('Systematics_Dstar_Standalone', SysList)
summaryOfLists(SysList)

process(analysis_main)

print(statistics)
