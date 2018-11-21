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
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('SLUntagged')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

SLpath = Path()

inputMdstList('MC9', fileList, path=SLpath)

stdPi('loose', path=SLpath)
stdK('loose', path=SLpath)
stdPi('all', path=SLpath)
stdE('all', path=SLpath)
stdMu('all', path=SLpath)

stdPi0s('loose', path=SLpath)  # for skim.standardlists.charm
stdPhotons('loose', path=SLpath)
stdKshorts(path=SLpath)

loadStdD0(path=SLpath)
loadStdDplus(path=SLpath)
loadStdDstar0(path=SLpath)
loadStdDstarPlus(path=SLpath)

# SL Skim
from skim.semileptonic import SemileptonicList
SLList = SemileptonicList(SLpath)
skimOutputUdst(skimCode, SLList, path=SLpath)

summaryOfLists(SLList, path=SLpath)


setSkimLogging(skim_path=SLpath)
process(SLpath)

# print out the summary
print(statistics)
