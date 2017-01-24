#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(*) h(*) skims
# P. Urquijo, 6/Jan/2015
#
######################################################
from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *

set_log_level(LogLevel.INFO)

filelist = [
    '/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/' +
    'prod00000796/s00/e0000/4S/r00000/ccbar/sub00/mdst_0001*.root'

]


inputMdstList('default', filelist)

loadStdCharged()
loadStdPi0()
loadStdKS()

fillParticleList('pi+:all', '')
fillParticleList('K+:all', '')

# B- to D(->Kshh)h- Skim
from BtoDh_Kshh_List import *


loadD()
BtoDhList = BsigToDhToKshhList()
skimOutputUdst('BtoDh_Kshh_skim_MC7_BG_PhaseIII', BtoDhList)
summaryOfLists(BtoDhList)

process(analysis_main)

# print out the summary
print(statistics)
