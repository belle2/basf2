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
gb2_setuprel = 'release-03-00-03'

hhpath = Path()

skimCode = encodeSkimName('BtoDh_hh')

fileList = get_test_file("mixedBGx1", "MC11")


inputMdstList('default', fileList, path=hhpath)


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
