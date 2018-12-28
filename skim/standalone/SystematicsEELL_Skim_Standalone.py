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
gb2_setuprel = 'release-03-00-00'

skimCode = encodeSkimName('SystematicsEELL')
import sys
import os
import glob

skimpath = Path()


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=skimpath)

stdE('all', path=skimpath)
stdMu('all', path=skimpath)

from skim.systematics import *
SysList = EELLList(skimpath)
skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)
