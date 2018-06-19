
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
from stdLightMesons import *
from stdPi0s import *
from stdV0s import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)

gb2_setuprel = 'release-02-00-00'

import sys
import os
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

stdPi0s('loose')
loadStdCharged()
loadStdSkimPhoton()
loadStdSkimPi0()
loadStdKS()
stdPhotons('loose')
loadStdLightMesons()

# Tau Skim
from TauGeneric_List import *
tauList = TauList()

skimOutputUdst(skimCode, tauList)
summaryOfLists(tauList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
