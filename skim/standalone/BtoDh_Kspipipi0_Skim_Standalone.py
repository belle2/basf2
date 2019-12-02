#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kspipipi0) h skims
# Minakshi Nayak, 24/Jan/2017
#
######################################################
from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPi0s import stdPi0s
from stdV0s import mergedKshorts, stdKshorts
from skim.standardlists.charm import loadStdD0, loadStdDplus, loadStdDStar0, loadStdDstarPlus
import skimExpertFunctions as expert
b2.set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

path = b2.Path()

skimCode = expert.encodeSkimName('BtoDh_Kspipipi0')

fileList = expert.get_test_file("mixedBGx1", "MC12")

ma.inputMdstList('default', fileList, path=path)


stdPi('all', path=path)
stdK('all', path=path)
stdKshorts(path=path)
loadStdSkimPi0(path=path)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import loadDkspipipi0, BsigToDhToKspipipi0List
loadDkspipipi0(path=path)
BtoDhList = BsigToDhToKspipipi0List(path=path)
expert.skimOutputUdst(skimCode, BtoDhList, path=path)
ma.summaryOfLists(BtoDhList, path=path)


expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(statistics)
