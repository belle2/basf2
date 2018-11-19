#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
import sys
import os
import glob
gb2_setuprel = 'release-02-00-01'
skimCode = encodeSkimName('LeptonicUntagged')


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

leppath = Path()

inputMdstList('MC9', fileList, path=leppath)

loadStdSkimPi0(path=leppath)
loadStdSkimPhoton(path=leppath)
loadStdCharged(path=leppath)
stdPi0s('loose', path=leppath)  # for stdCharm.py
stdPhotons('loose', path=leppath)
loadStdKS(path=leppath)

loadStdD0(path=leppath)
loadStdDplus(path=leppath)
loadStdDstar0(path=leppath)
loadStdDstarPlus(path=leppath)

# SL Skim
from skim.leptonic import LeptonicList

lepList = LeptonicList(path=leppath)
skimOutputUdst(skimCode, lepList, path=leppath)

summaryOfLists(lepList, path=leppath)


setSkimLogging(skim_path=leppath)
process(leppath)

# print out the summary
print(statistics)
