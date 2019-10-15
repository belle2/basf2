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
from stdCharged import stdPi, stdK
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

path = Path()

skimCode = encodeSkimName('BtoDh_Kspipipi0')

fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=path)


stdPi('all', path=path)
stdK('all', path=path)
stdKshorts(path=path)
loadStdSkimPi0(path=path)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import loadDkspipipi0, BsigToDhToKspipipi0List
loadDkspipipi0(path=path)
BtoDhList = BsigToDhToKspipipi0List(path=path)
skimOutputUdst(skimCode, BtoDhList, path=path)
summaryOfLists(BtoDhList, path=path)


setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
