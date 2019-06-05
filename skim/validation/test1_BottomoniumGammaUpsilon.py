#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__author__ = "S. Spataro && S. Jia"

from basf2 import *
from modularAnalysis import *
from stdPi0s import *
from stdPhotons import *
from stdCharged import *
from skimExpertFunctions import encodeSkimName, setSkimLogging

# create a new path
BottomoniumGammaUpsilonskimpath = Path()

fileList = ['../BottomoniumGammaUpsilon.dst.root']

inputMdstList('default', fileList, path=BottomoniumGammaUpsilonskimpath)

# use standard final state particle lists
stdPi('loose', path=BottomoniumGammaUpsilonskimpath)
stdE('loose', path=BottomoniumGammaUpsilonskimpath)
stdMu('loose', path=BottomoniumGammaUpsilonskimpath)

stdPhotons('loose', path=BottomoniumGammaUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList(path=BottomoniumGammaUpsilonskimpath)
skimOutputUdst('../BottomoniumGammaUpsilon.udst.root', YList, path=BottomoniumGammaUpsilonskimpath)
summaryOfLists(YList, path=BottomoniumGammaUpsilonskimpath)


setSkimLogging(path=BottomoniumGammaUpsilonskimpath)
process(BottomoniumGammaUpsilonskimpath)

# print out the summary
print(statistics)
