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
from skimExpertFunctions import encodeSkimName

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-01-00-00'

import sys
import os
import glob

fileList = [
    '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900520000_0.root',
    '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900420000_*.root']


inputMdstList('default', fileList)

loadStdCharged()

from SystematicsRadMuMu_List import *
SysList = SystematicsList()
skimCode = encodeSkimName('SystematicsEELL')
skimOutputUdst(skimCode, SysList)
summaryOfLists(SysList)

setSkimLogging()
process(analysis_main)

print(statistics)
