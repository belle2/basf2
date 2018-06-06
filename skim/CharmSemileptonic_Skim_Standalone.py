#!/usr/bin/evn python3
# -*- coding: utf-8 -*-

##################################################
#
# Charm SL skims
# J. Bennett, 5/Oct/2016
#
##################################################

from basf2 import *
from modularAnalysis import *
from stdFSParticles import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'

import os
import sys
import glob
scriptName = sys.argv[0]
skimListName = scriptName[:-19]
skimCode = encodeSkimName(skimListName)
print(skimListName)
print(skimCode)
fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_0005*_prod00000198_task000005*.root'
     ]

inputMdstList('default', fileList)


stdPi()
stdK()
loadStdSkimPi0()
reconstructDecay('K_S0:all -> pi-:95eff pi+:95eff', '0.4 < M < 0.6', 1, True, analysis_main)
vertexKFit('K_S0:all', 0.0)
applyCuts('K_S0:all', '0.477614 < M < 0.517614')

fillParticleList('e+:std', 'electronID > 0.1 and chiProb > 0.001 and p > 0.25', True, analysis_main)

fillParticleList('mu+:std', 'muonID > 0.1 and chiProb > 0.001 and p > 0.25', True, analysis_main)

# CSL Skim
from CharmSemileptonic_List import *
CSLList = CharmSemileptonicList()
skimOutputUdst(skimCode, CSLList)
summaryOfLists(CSLList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
