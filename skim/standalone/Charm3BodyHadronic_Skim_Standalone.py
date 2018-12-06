#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdMu, stdE, stdMu
from stdV0s import *
from stdPi0s import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'
set_log_level(LogLevel.INFO)
import sys
import os
import glob
skimCode = encodeSkimName('Charm3BodyHadronic')

c3bhpath = Path()
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=c3bhpath)


loadStdSkimPhoton(path=c3bhpath)
loadStdSkimPi0(path=c3bhpath)
stdPi('loose', path=c3bhpath)
stdK('loose', path=c3bhpath)
stdE('loose', path=c3bhpath)
stdMu('loose', path=c3bhpath)
stdPi('all', path=c3bhpath)
stdK('all', path=c3bhpath)
stdE('all', path=c3bhpath)
stdMu('all', path=c3bhpath)
stdKshorts(path=c3bhpath)

from skim.charm import DstToD0PiD0ToHpJmPi0
DstToD0PiD0ToHpJmPi0List = DstToD0PiD0ToHpJmPi0(c3bhpath)
skimOutputUdst(skimCode, DstToD0PiD0ToHpJmPi0List, path=c3bhpath)

summaryOfLists(DstToD0PiD0ToHpJmPi0List, path=c3bhpath)


setSkimLogging(path=c3bhpath)
process(c3bhpath)

print(statistics)
