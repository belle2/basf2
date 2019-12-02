#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(*) h(*) skims
# P. Urquijo, 6/Jan/2015
#
######################################################
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdV0s import stdKshorts
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

path = b2.Path()

skimCode = expert.encodeSkimName('BtoDh_Kshh')


fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=path)


stdPi('all', path=path)
stdK('all', path=path)
stdKshorts(path=path)

# B- to D(->Kshh)h- Skim
from skim.btocharm import loadDkshh, BsigToDhToKshhList
loadDkshh(path=path)
BtoDhList = BsigToDhToKshhList(path=path)
expert.skimOutputUdst(skimCode, BtoDhList, path=path)
ma.summaryOfLists(BtoDhList, path=path)


expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(statistics)
