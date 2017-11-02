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
gb2_setuprel = 'build-2017-10-16'
import sys
import os
import glob
fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]


inputMdstList('default', fileList)


loadStdCharged()
loadStdKS()
stdPhotons('loose')
stdPi0s('loose')
loadStdSkimPi0()
loadStdLightMesons()
# Had Skim
from CharmlessHad_List import *
HadList = CharmlessHadList()
skimOutputUdst('CharmlessHad', HadList)
summaryOfLists(HadList)

process(analysis_main)

# print out the summary
print(statistics)
