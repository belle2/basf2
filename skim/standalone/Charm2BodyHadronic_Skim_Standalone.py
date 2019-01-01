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
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-00'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('Charm2BodyHadronic')

c2bhpath = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=c2bhpath)

stdPi('loose', path=c2bhpath)
stdK('loose', path=c2bhpath)
stdPi('all', path=c2bhpath)
stdK('all', path=c2bhpath)
stdE('all', path=c2bhpath)

from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(c2bhpath)

skimOutputUdst(skimCode, DstToD0PiD0ToHpJmList, path=c2bhpath)
summaryOfLists(DstToD0PiD0ToHpJmList, path=c2bhpath)


setSkimLogging(path=c2bhpath)
process(c2bhpath)

print(statistics)
