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
gb2_setuprel = 'build-2017-10-16'

import sys
import os
import glob

fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]

inputMdstList('default', fileList)


loadStdCharged()

from SystematicsJpsimumu_List import *
SysList = SystematicsList()
skimOutputUdst('SystematicsJpsimumu', SysList)
summaryOfLists(SysList)

process(analysis_main)

print(statistics)
