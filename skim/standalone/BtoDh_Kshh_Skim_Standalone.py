#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(*) h(*) skims
# P. Urquijo, 6/Jan/2015
#
######################################################
from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdV0s import *
from skimExpertFunctions import encodeSkimName, setSkimLogging

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'

mypath = Path()

skimCode = encodeSkimName('BtoDh_Kshh')


fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=mypath)


stdPi('all', path=mypath)
stdK('all', path=mypath)
stdKshorts(path=mypath)

# B- to D(->Kshh)h- Skim
from skim.btocharm import loadDkshh, BsigToDhToKshhList
loadDkshh(path=mypath)
BtoDhList = BsigToDhToKshhList(path=mypath)
skimOutputUdst(skimCode, BtoDhList, path=mypath)
summaryOfLists(BtoDhList, path=mypath)


setSkimLogging()
process(mypath)

# print out the summary
print(statistics)
