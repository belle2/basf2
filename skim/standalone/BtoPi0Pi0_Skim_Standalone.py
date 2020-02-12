# !/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B0 -> pi0 pi0
#
# K. Smith
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
from stdPi0s import loadStdSkimPi0

import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)

gb2_setuprel = "release-04-00-00"

skimCode = expert.encodeSkimName('BtoPi0Pi0')

path = b2.Path()
fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=path)

# load particle lists
loadStdSkimPi0(path=path)

# Had Skim
from skim.btocharmless import BtoPi0Pi0List
Pi0Pi0List = BtoPi0Pi0List(path=path)
expert.skimOutputUdst(skimCode, Pi0Pi0List, path=path)

ma.summaryOfLists(Pi0Pi0List, path=path)


expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
