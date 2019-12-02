#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Single photon skim standalone for the dark photon analysis in WG8: Dark/low multi.
Physics channel: ee → A'γ; A' → invisible; Skim LFN code:   18020100
"""

__author__ = "Sam Cunliffe"


from basf2 import process, statistics, Path
from modularAnalysis import inputMdstList, skimOutputUdst, summaryOfLists
from stdCharged import stdE, stdMu
from stdPhotons import stdPhotons
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

# create a path
darkskimpath = b2.Path()

# test input file
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=darkskimpath)
stdPhotons('all', path=darkskimpath)
stdE('all', path=darkskimpath)
stdMu('all', path=darkskimpath)

# dark photon skim
from skim.dark import SinglePhotonDarkList
darklist = SinglePhotonDarkList(path=darkskimpath)
skimCode = expert.encodeSkimName('SinglePhotonDark')
print("Single photon dark skim:", skimCode)
expert.skimOutputUdst(skimCode, darklist, path=darkskimpath)
ma.summaryOfLists(darklist, path=darkskimpath)

# suppress noisy modules, and then process
expert.setSkimLogging(path=darkskimpath)
b2.process(darkskimpath)

# print out the summary
print(statistics)
