#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__author__ = "S. Spataro && S. Jia"

from basf2 import *
from modularAnalysis import *
from stdPi0s import *
from stdPhotons import *
from stdCharged import *
from skimExpertFunctions import encodeSkimName, setSkimLogging
gb2_setuprel = 'release-02-00-01'

# create a new path
BottomoniumGammaUpsilonskimpath = Path()

fileList = ['../BottomoniumGammaUpsilon.dst.root']

inputMdstList('MC9', fileList, path=BottomoniumGammaUpsilonskimpath)

# use standard final state particle lists
stdPi('loose', path=BottomoniumGammaUpsilonskimpath)
stdE('loose', path=BottomoniumGammaUpsilonskimpath)
stdMu('loose', path=BottomoniumGammaUpsilonskimpath)

stdPhotons('loose', path=BottomoniumGammaUpsilonskimpath)

inputMdstList('MC9', fileList, path=BottomoniumGammaUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList(path=BottomoniumGammaUpsilonskimpath)
skimOutputUdst('../BottomoniumGammaUpsilon.udst.root', YList, path=BottomoniumGammaUpsilonskimpath)
summaryOfLists(YList, path=BottomoniumGammaUpsilonskimpath)


setSkimLogging(path=BottomoniumGammaUpsilonskimpath)
process(BottomoniumGammaUpsilonskimpath)

# print out the summary
print(statistics)
