#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Hulya Atmacan 2016/Oct/25
#
######################################################

from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK
import skimExpertFunctions as expert
b2.set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

path = b2.Path()

skimCode = expert.encodeSkimName('BtoDh_hh')

fileList = expert.get_test_file("mixedBGx1", "MC12")


ma.inputMdstList('default', fileList, path=path)


# create and fill pion and kaon ParticleLists
stdPi('all', path=path)
stdK('all', path=path)

# B+ to D(->h+h-)h+ Skim
from skim.btocharm import loadD0bar, BsigToDhTohhList
loadD0bar(path=path)
BtoDhList = BsigToDhTohhList(path=path)
expert.skimOutputUdst(skimCode, BtoDhList, path=path)
ma.summaryOfLists(BtoDhList, path=path)


expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(statistics)
