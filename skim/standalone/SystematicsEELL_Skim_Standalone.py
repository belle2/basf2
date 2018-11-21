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


fileList = [
        '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900520000_0.root',
        '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900420000_*.root'
    ]


inputMdstList('MC9', fileList)

stdE('all')
stdMu('all')


from skim.systematics import *
SysList = EELLList()
skimOutputUdst(skimCode, SysList)
summaryOfLists(SysList)

setSkimLogging()
process(analysis_main)

print(statistics)
