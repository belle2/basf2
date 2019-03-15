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
from skimExpertFunctions import encodeSkimName, setSkimLogging
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'

kspipipi0path = Path()

skimCode = encodeSkimName('BtoDh_Kspipipi0')

fileList = get_test_file("mixedBGx1", "MC11")

inputMdstList('default', fileList, path=kspipipi0path)


stdPi('all', path=kspipipi0path)
stdK('all', path=kspipipi0path)
stdKshorts(path=kspipipi0path)
loadStdSkimPi0(path=kspipipi0path)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import loadDkspipipi0, BsigToDhToKspipipi0List
loadDkspipipi0(path=kspipipi0path)
BtoDhList = BsigToDhToKspipipi0List(path=kspipipi0path)
skimOutputUdst(skimCode, BtoDhList, path=kspipipi0path)
summaryOfLists(BtoDhList, path=kspipipi0path)


setSkimLogging()
process(kspipipi0path)

# print out the summary
print(statistics)
