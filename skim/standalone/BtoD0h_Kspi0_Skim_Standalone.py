#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kspi0) h skims
# Minakshi Nayak, 24/Jan/2017
#
######################################################
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPi0s import loadStdSkimPi0
from stdV0s import stdKshorts
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

path = b2.Path()

skimCode = expert.encodeSkimName('BtoD0h_Kspi0')

fileList = expert.get_test_file("MC12_mixedBGx1")


ma.inputMdstList('default', fileList, path=path)


stdPi('all', path=path)
stdK('all', path=path)
loadStdSkimPi0(path=path)
stdKshorts(path=path)

# B+ to anti-D0(->Kspi0)h+ Skim
from skim.btocharm import BsigToD0hToKspi0List
from skim.standardlists.charm import loadD0_Kspi0_sideband

loadD0_Kspi0_sideband(path=path)
BtoD0h_Kspi0_List = BsigToD0hToKspi0List(path=path)
expert.skimOutputUdst(skimCode, BtoD0h_Kspi0_List, path=path)
ma.summaryOfLists(BtoD0h_Kspi0_List, path=path)


expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
