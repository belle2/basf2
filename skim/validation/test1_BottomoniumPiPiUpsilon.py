#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../BottomoniumPiPiUpsilon.dst.root</input>
  <output>../BottomoniumPiPiUpsilon.udst.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
  <interval>nightly</interval>
</header>
"""

__author__ = "S. Spataro && S. Jia"

from basf2 import *
from modularAnalysis import *
from stdPi0s import *
from stdPhotons import *
from stdCharged import *
from skimExpertFunctions import encodeSkimName, setSkimLogging

# create a new path
BottomoniumPiPiUpsilonskimpath = Path()

fileList = ['../BottomoniumPiPiUpsilon.dst.root']

inputMdstList('default', fileList, path=BottomoniumPiPiUpsilonskimpath)

# use standard final state particle lists
stdPi('loose', path=BottomoniumPiPiUpsilonskimpath)
stdE('loose', path=BottomoniumPiPiUpsilonskimpath)
stdMu('loose', path=BottomoniumPiPiUpsilonskimpath)

stdPhotons('loose', path=BottomoniumPiPiUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList(path=BottomoniumPiPiUpsilonskimpath)
skimOutputUdst('../BottomoniumPiPiUpsilon.udst.root', YList, path=BottomoniumPiPiUpsilonskimpath)
summaryOfLists(YList, path=BottomoniumPiPiUpsilonskimpath)


setSkimLogging(path=BottomoniumPiPiUpsilonskimpath)
process(BottomoniumPiPiUpsilonskimpath)

# print out the summary
print(statistics)
