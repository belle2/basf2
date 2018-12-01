#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Double charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)


gb2_setuprel = 'release-02-00-01'
import os
import sys
import glob
skimCode = encodeSkimName('DoubleCharm')
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

path = Path()
inputMdstList('MC9', fileList, path=path)
stdPi('all', path=path)
stdPi('loose', path=path)
stdK('loose', path=path)
stdKshorts(path=path)
loadStdSkimPi0(path=path)
loadStdSkimPhoton(path=path)
stdPi0s('loose', path=path)
stdPhotons('loose', path=path)
loadStdD0(path=path)
loadStdDplus(path=path)
loadStdDstar0(path=path)
loadStdDstarPlus(path=path)

# Double Charm Skim
from skim.btocharm import *
DCList = DoubleCharmList(path=path)
skimOutputUdst(skimCode, DCList, path=path)
summaryOfLists(DCList, path=path)
setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
