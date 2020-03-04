#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(Kspipipi0) h skims
# Minakshi Nayak, 24/Jan/2017
#
######################################################
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons
from stdV0s import stdKshorts
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

path = b2.Path()

skimCode = expert.encodeSkimName('BtoD0h_Kspipipi0')

fileList = expert.get_test_file("MC12_mixedBGx1")

ma.inputMdstList('default', fileList, path=path)

loadStdPi0ForBToHadrons(path=path)
stdPi('all', path=path)
stdK('all', path=path)
stdKshorts(path=path)

# B+ to anti-D0(->Kspipipi0)h+ Skim
from skim.btocharm import BsigToD0hToKspipipi0List
from skim.standardlists.charm import loadD0_Kspipipi0

loadD0_Kspipipi0(path=path)
BtoD0h_Kspipipi0_List = BsigToD0hToKspipipi0List(path=path)
expert.skimOutputUdst(skimCode, BtoD0h_Kspipipi0_List, path=path)
ma.summaryOfLists(BtoD0h_Kspipipi0_List, path=path)


expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
