#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro, 25/Jul/2016
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdPi0s import *
from stdPhotons import *
from stdCharged import *
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
import sys

# create a new path
BottomoniumUpsilonskimpath = b2.Path()

skimCode = expert.encodeSkimName('BottomoniumUpsilon')

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=BottomoniumUpsilonskimpath)

# use standard final state particle lists
stdPi('loose', path=BottomoniumUpsilonskimpath)
stdE('loose', path=BottomoniumUpsilonskimpath)
stdMu('loose', path=BottomoniumUpsilonskimpath)

stdPhotons('loose', path=BottomoniumUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList(path=BottomoniumUpsilonskimpath)
expert.skimOutputUdst(skimCode, YList, path=BottomoniumUpsilonskimpath)
ma.summaryOfLists(YList, path=BottomoniumUpsilonskimpath)


expert.setSkimLogging(path=BottomoniumUpsilonskimpath)
b2.process(BottomoniumUpsilonskimpath)

# print out the summary
print(statistics)
