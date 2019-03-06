#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kspi0) h skims
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
gb2_setuprel = 'release-03-00-03'

kspi0path = Path()

skimCode = encodeSkimName('BtoDh_Kspi0')

fileList = get_test_file("mixedBGx1", "MC11")


inputMdstList('default', fileList, path=kspi0path)


stdPi('all', path=kspi0path)
stdK('all', path=kspi0path)
loadStdSkimPi0(path=kspi0path)
stdKshorts(path=kspi0path)

# B- to D(->Kspi0)h- Skim
from skim.btocharm import loadDkspi0, BsigToDhToKspi0List
loadDkspi0(path=kspi0path)
BtoDhList = BsigToDhToKspi0List(path=kspi0path)
skimOutputUdst(skimCode, BtoDhList, path=kspi0path)
summaryOfLists(BtoDhList, path=kspi0path)


setSkimLogging()
process(kspi0path)

# print out the summary
print(statistics)
