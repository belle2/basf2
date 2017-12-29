#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kspipipi0) h skims
# Minakshi Nayak, 24/Jan/2017
#
######################################################
from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'build-2017-10-16'

fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]


inputMdstList('default', fileList)


loadStdCharged()
loadStdKS()
loadStdSkimPi0()
fillParticleList('pi+:all', '')
fillParticleList('K+:all', '')

# B- to D(->Kspipipi0)h- Skim
from BtoDh_Kspipipi0_List import *


loadD()
BtoDhList = BsigToDhToKspipipi0List()
skimOutputUdst('BtoDh_Kspipipi0', BtoDhList)
summaryOfLists(BtoDhList)

process(analysis_main)

# print out the summary
print(statistics)
