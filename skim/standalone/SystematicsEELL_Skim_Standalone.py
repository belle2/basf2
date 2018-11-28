#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for four-lepton events
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging


set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'

skimCode = encodeSkimName('SystematicsEELL')
import sys
import os
import glob

skimpath = Path()

fileList = [
    '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900520000_0.root',
    '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900420000_*.root'
]


inputMdstList('MC9', fileList, path=skimpath)

stdE('all', path=skimpath)
stdMu('all', path=skimpath)

from skim.systematics import *
SysList = EELLList(skimpath)
skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

setSkimLogging(skim_path=skimpath)
process(skimpath)

print(statistics)
