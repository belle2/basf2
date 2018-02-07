#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Had skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdLightMesons import *
from stdV0s import *
from stdPi0s import *
import sys
import os
import glob


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)

loadStdSkimPi0()
loadStdCharged()
loadStdKS()
stdPi0s('loose')
stdPhotons('loose')
loadStdLightMesons()
# Had Skim
from CharmlessHad_List import *
HadList = CharmlessHadList()
skimOutputUdst('CharmlessHad', HadList)

summaryOfLists(HadList)

process(analysis_main)

# print out the summary
print(statistics)
