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
from stdCharged import stdPi, stdK
from skimExpertFunctions import encodeSkimName, setSkimLogging

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-00'

hhpath = Path()

skimCode = encodeSkimName('BtoDh_hh')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList, path=hhpath)


# create and fill pion and kaon ParticleLists
stdPi('all', path=hhpath)
stdK('all', path=hhpath)

# B+ to D(->h+h-)h+ Skim
from skim.btocharm import loadD0bar, BsigToDhTohhList
loadD0bar(path=hhpath)
BtoDhList = BsigToDhTohhList(path=hhpath)
skimOutputUdst(skimCode, BtoDhList, path=hhpath)
summaryOfLists(BtoDhList, path=hhpath)


setSkimLogging()
process(hhpath)

# print out the summary
print(statistics)
