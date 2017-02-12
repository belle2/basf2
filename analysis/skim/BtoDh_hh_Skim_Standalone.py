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

set_log_level(LogLevel.INFO)

filelist = [
           '/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/' +
           'MC7/prod00000765/s00/e0000/4S/r00000/signal/sub00/mdst_000001_prod00000765_task00000001.root'
           ]

inputMdstList('default', filelist)

# create and fill pion and kaon ParticleLists
# second argument are the selection criteria: '' means no cut, take all
fillParticleList('pi+:all', '')
fillParticleList('K+:all', '')


# B+ to D(->h+h-)h+ Skim
from BtoDh_hh_List import *

loadD0bar()
BtoDhList = BsigToDhTohhList()

skimOutputUdst('BtoDh_hh_skim_MC7_1213030000_BG', BtoDhList)
summaryOfLists(BtoDhList)

process(analysis_main)

# print out the summary
print(statistics)
