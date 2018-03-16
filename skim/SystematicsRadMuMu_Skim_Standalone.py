#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for radiative muon pairs
# Torben Ferber (torben.ferber@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-01-00-00'

import sys
import os
import glob

fileList = [
    ''
]


inputMdstList('default', fileList)

loadStdCharged()

from SystematicsRadMuMu_List import *
SysList = SystematicsList()
skimOutputUdst('SystematicsRadMuMu', SysList)
summaryOfLists(SysList)

process(analysis_main)

print(statistics)
