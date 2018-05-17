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
from stdV0s import *

from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
scriptName = sys.argv[0]
skimListName = scriptName[:-19]
skimCode = encodeSkimName(skimListName)
print(skimListName)
print(skimCode)

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('default', fileList)

stdKshorts()
loadStdCharged()
from Charm3BodyHadronic2_List import *


DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs()
skimOutputUdst(skimCode, DstToD0PiD0ToHpHmKsList)

summaryOfLists(DstToD0PiD0ToHpHmKsList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)


for module in analysis_main.modules():
    if module.type() == "ParticleVertexFitter":
        module.set_log_level(LogLevel.ERROR)

process(analysis_main)

print(statistics)
