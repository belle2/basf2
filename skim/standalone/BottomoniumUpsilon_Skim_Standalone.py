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
gb2_setuprel = 'release-03-00-00'
import sys
import os
import glob

# create a new path
BottomoniumUpsilonskimpath = Path()

skimCode = encodeSkimName('BottomoniumUpsilon')
fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]

# use standard final state particle lists
stdPi('loose', path=BottomoniumUpsilonskimpath)
stdE('loose', path=BottomoniumUpsilonskimpath)
stdMu('loose', path=BottomoniumUpsilonskimpath)

stdPhotons('loose', path=BottomoniumUpsilonskimpath)

inputMdstList('MC9', fileList, path=BottomoniumUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList(path=BottomoniumUpsilonskimpath)
skimOutputUdst(skimCode, YList, path=BottomoniumUpsilonskimpath)
summaryOfLists(YList, path=BottomoniumUpsilonskimpath)


setSkimLogging(path=BottomoniumUpsilonskimpath)
process(BottomoniumUpsilonskimpath)

# print out the summary
print(statistics)
