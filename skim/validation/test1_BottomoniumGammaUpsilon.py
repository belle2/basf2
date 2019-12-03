#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../BottomoniumGammaUpsilon.dst.root</input>
  <output>../BottomoniumGammaUpsilon.udst.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
  <interval>nightly</interval>
</header>
"""

__author__ = "S. Spataro && S. Jia"

import basf2 as b2
import modularAnalysis as ma
from stdPi0s import *
from stdPhotons import *
from stdCharged import *
import skimExpertFunctions as expert

# create a new path
BottomoniumGammaUpsilonskimpath = b2.Path()

fileList = ['../BottomoniumGammaUpsilon.dst.root']

ma.inputMdstList('default', fileList, path=BottomoniumGammaUpsilonskimpath)

# use standard final state particle lists
stdPi('loose', path=BottomoniumGammaUpsilonskimpath)
stdE('loose', path=BottomoniumGammaUpsilonskimpath)
stdMu('loose', path=BottomoniumGammaUpsilonskimpath)

stdPhotons('loose', path=BottomoniumGammaUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList(path=BottomoniumGammaUpsilonskimpath)
expert.skimOutputUdst('../BottomoniumGammaUpsilon.udst.root', YList, path=BottomoniumGammaUpsilonskimpath)
ma.summaryOfLists(YList, path=BottomoniumGammaUpsilonskimpath)


expert.setSkimLogging(path=BottomoniumGammaUpsilonskimpath)
b2.process(BottomoniumGammaUpsilonskimpath)

# print out the summary
print(b2.statistics)
