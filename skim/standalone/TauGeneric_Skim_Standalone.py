#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau generic skims
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'

skimCode = encodeSkimName('TauGeneric')
import sys
import os
import glob

taugenericskim = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# Tau Skim
from skim.taupair import *
tauList = TauList(path=taugenericskim)

skimOutputUdst(skimCode, tauList, path=taugenericskim)
summaryOfLists(tauList, path=taugenericskim)

setSkimLogging(path=taugenericskim)
process(taugenericskim)

# print out the summary
print(statistics)
