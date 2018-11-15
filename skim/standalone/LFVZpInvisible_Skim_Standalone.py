#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skims for LFV Z'
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from skimExpertFunctions import encodeSkimName, setSkimLogging

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'

skimCode = encodeSkimName('LFVZpInvisible')
import sys
import os
import glob

lfvzppath = Path()

fileList = [
    '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900520000_0.root',
    '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900420000_*.root'
]
inputMdstList('MC9', fileList, path=lfvzppath)
loadStdCharged(path=lfvzppath)

from skim.dark import LFVZpInvisibleList
SysList = LFVZpInvisibleList(path=lfvzppath)
skimOutputUdst(skimCode, SysList, path=lfvzppath)
summaryOfLists(SysList, path=lfvzppath)

setSkimLogging(skim_path=lfvzppath)
process(lfvzppath)

print(statistics)
