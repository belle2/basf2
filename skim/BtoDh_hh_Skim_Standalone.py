#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Hulya Atmacan 2016/Oct/25
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-01-00-00'

fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]


inputMdstList('default', fileList)


# create and fill pion and kaon ParticleLists
# second argument are the selection criteria: '' means no cut, take all
loadStdCharged()

# B+ to D(->h+h-)h+ Skim
from BtoDh_hh_List import *

loadD0bar()
BtoDhList = BsigToDhTohhList()

skimOutputUdst('BtoDh_hh', BtoDhList)

summaryOfLists(BtoDhList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
