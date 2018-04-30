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
from stdPi0s import *
from skimExpertFunction import *
gb2_setuprel = 'release-02-00-00'
set_log_level(LogLevel.INFO)

import os
import sys
import glob
scriptName = sys.argv[0]
skimListName = scriptName[:-19]
outputLFN = getOutputLFN(skimListName)
print(skimListName)
print(outputLFN)

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('default', fileList)


loadStdSkimPi0()
stdKshorts()
loadStdCharged()


from Charm3BodyHadronicD0_List import *
D0ToHpJmPi0List = D0ToHpJmPi0()
skimOutputUdst(outputLFN, D0ToHpJmPi0List)

summaryOfLists(D0ToHpJmPi0List)

process(analysis_main)

print(statistics)
