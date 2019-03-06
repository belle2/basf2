#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro, 25/Jul/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPi0s import *
from stdPhotons import *
from stdCharged import *
from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-03'
import sys

# create a new path
BottomoniumUpsilonskimpath = Path()

skimCode = encodeSkimName('BottomoniumUpsilon')

fileList = get_test_file("mixedBGx1", "MC11")

# use standard final state particle lists
stdPi('loose', path=BottomoniumUpsilonskimpath)
stdE('loose', path=BottomoniumUpsilonskimpath)
stdMu('loose', path=BottomoniumUpsilonskimpath)

stdPhotons('loose', path=BottomoniumUpsilonskimpath)

inputMdstList('default', fileList, path=BottomoniumUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList(path=BottomoniumUpsilonskimpath)
skimOutputUdst(skimCode, YList, path=BottomoniumUpsilonskimpath)
summaryOfLists(YList, path=BottomoniumUpsilonskimpath)


setSkimLogging(path=BottomoniumUpsilonskimpath)
process(BottomoniumUpsilonskimpath)

# print out the summary
print(statistics)
