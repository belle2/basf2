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
from stdCharm import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-01-00-00'


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)


loadStdCharged()
loadStdKS()
loadStdSkimPi0()

# B- to D(->Kspipipi0)h- Skim
from BtoDh_Kspipipi0_List import *
loadD()
BtoDhList = BsigToDhToKspipipi0List()
skimOutputUdst('BtoDh_Kspipipi0', BtoDhList)
summaryOfLists(BtoDhList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
