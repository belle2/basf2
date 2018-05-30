#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 29/Sep/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from stdLightMesons import *
from stdDiLeptons import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
set_log_level(LogLevel.INFO)


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

loadStdSkimPi0()
loadStdSkimPhoton()
stdPi0s('loose')
loadStdCharged()
stdPhotons('loose')
loadStdKS()
loadStdDiLeptons(True)
loadStdLightMesons()
cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4')
# TCPV Skim
from TCPV_List import *
tcpvList = TCPVList()
skimOutputUdst(skimCode, tcpvList)
summaryOfLists(tcpvList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
