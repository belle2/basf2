# !/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B0 -> pi0 pi0
#
# K. Smith
#
#######################################################

from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
from stdPi0s import *

from skimExpertFunctions import *
set_log_level(LogLevel.INFO)

gb2_setuprel = "release-01-00-00"


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)


# load particle lists
loadStdSkimPi0()

# Had Skim
from BtoPi0Pi0_List import *
Pi0Pi0List = BtoPi0Pi0List()
skimOutputUdst('BtoPi0Pi0', Pi0Pi0List)

summaryOfLists(Pi0Pi0List)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
