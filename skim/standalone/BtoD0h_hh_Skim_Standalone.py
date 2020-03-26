#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Hulya Atmacan 2016/Oct/25
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

path = b2.Path()

skimCode = expert.encodeSkimName('BtoD0h_hh')

fileList = expert.get_test_file("MC12_mixedBGx1")


ma.inputMdstList('default', fileList, path=path)


# create and fill pion and kaon ParticleLists
stdPi('all', path=path)
stdK('all', path=path)

# B+ to anti-D0(->h+h-)h+ Skim
from skim.btocharm import BsigToD0hTohhList
from skim.standardlists.charm import loadD0_hh_loose

loadD0_hh_loose(path=path)
BtoD0h_hh_List = BsigToD0hTohhList(path=path)
expert.skimOutputUdst(skimCode, BtoD0h_hh_List, path=path)
ma.summaryOfLists(BtoD0h_hh_List, path=path)


expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
