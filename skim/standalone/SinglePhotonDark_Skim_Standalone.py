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
from skimExpertFunctions import setSkimLogging, encodeSkimName

gb2_setuprel = 'release-03-00-03'

# create a path
darkskimpath = Path()

# test input file
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=darkskimpath)
stdPhotons('all', path=darkskimpath)
stdE('all', path=darkskimpath)
stdMu('all', path=darkskimpath)

# dark photon skim
from skim.dark import SinglePhotonDarkList
darklist = SinglePhotonDarkList(path=darkskimpath)
skimCode = encodeSkimName('SinglePhotonDark')
print("Single photon dark skim:", skimCode)
skimOutputUdst(skimCode, darklist, path=darkskimpath)
summaryOfLists(darklist, path=darkskimpath)

# suppress noisy modules, and then process
setSkimLogging(path=darkskimpath)
process(darkskimpath)

# print out the summary
print(statistics)
