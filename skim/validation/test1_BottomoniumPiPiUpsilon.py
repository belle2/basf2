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

import basf2 as b2
import modularAnalysis as ma
from stdPhotons import stdPhotons
from stdCharged import stdE, stdMu, stdPi
import skimExpertFunctions as expert

# create a new path
BottomoniumPiPiUpsilonskimpath = b2.Path()

fileList = ['../BottomoniumPiPiUpsilon.dst.root']

ma.inputMdstList('default', fileList, path=BottomoniumPiPiUpsilonskimpath)

# use standard final state particle lists
stdPi('loose', path=BottomoniumPiPiUpsilonskimpath)
stdE('loose', path=BottomoniumPiPiUpsilonskimpath)
stdMu('loose', path=BottomoniumPiPiUpsilonskimpath)

stdPhotons('loose', path=BottomoniumPiPiUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import UpsilonList
YList = UpsilonList(path=BottomoniumPiPiUpsilonskimpath)
expert.skimOutputUdst('../BottomoniumPiPiUpsilon.udst.root', YList, path=BottomoniumPiPiUpsilonskimpath)
ma.summaryOfLists(YList, path=BottomoniumPiPiUpsilonskimpath)


expert.setSkimLogging(path=BottomoniumPiPiUpsilonskimpath)
b2.process(BottomoniumPiPiUpsilonskimpath)

# print out the summary
print(b2.statistics)
