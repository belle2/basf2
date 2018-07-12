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
from stdCharged import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
skimCode = encodeSkimName('Charm2BodyHadronic')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList)


loadStdCharged()

from Charm2BodyHadronic_List import *

DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm()
skimOutputUdst(skimCode, DstToD0PiD0ToHpJmList)

summaryOfLists(DstToD0PiD0ToHpJmList)


setSkimLogging()
process(analysis_main)

print(statistics)
