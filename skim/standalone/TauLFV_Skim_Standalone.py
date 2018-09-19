
# -*- coding: utf-8 -*-

#######################################################
#
# Tau skims
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdLightMesons import *
from stdPi0s import *
from stdV0s import *
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-00'

skimCode = encodeSkimName('TauLFV')

import sys
import os
import glob
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)

# Tau Skim
from skim.taupair import *
tauList = TauLFVList()

skimOutputUdst(skimCode, tauList)
summaryOfLists(tauList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
