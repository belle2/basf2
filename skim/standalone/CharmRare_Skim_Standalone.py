#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# Charm skims
# P. Urquijo, 6/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-00'
set_log_level(LogLevel.INFO)
import os
import sys
import glob
skimCode = encodeSkimName('CharmRare')

crpath = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('MC9', fileList, path=crpath)


loadStdSkimPi0(path=crpath)
loadStdSkimPhoton(path=crpath)
stdMu('loose', path=crpath)
stdE('loose', path=crpath)

from skim.charm import CharmRareList
CharmRareList = CharmRareList(crpath)
skimOutputUdst(skimCode, CharmRareList, path=crpath)
summaryOfLists(CharmRareList, path=crpath)

setSkimLogging(path=crpath)
process(crpath)

print(statistics)
