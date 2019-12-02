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
from ROOT import Belle2
import modularAnalysis as ma
from stdPi0s import *

import skimExpertFunctions as expert
b2.set_log_level(LogLevel.INFO)

gb2_setuprel = "release-04-00-00"

import os
import sys
import glob
skimCode = expert.encodeSkimName('BtoPi0Pi0')

path = b2.Path()
fileList = expert.get_test_file("mixedBGx1", "MC12")
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
print(statistics)
