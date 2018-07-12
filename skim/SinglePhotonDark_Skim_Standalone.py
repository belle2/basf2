#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Single photon skim standalone for the dark photon analysis in WG8: Dark/low multi.
Physics channel: ee → A'γ; A' → invisible; Skim LFN code:   18020100
"""

__author__ = "Sam Cunliffe"


from basf2 import process, statistics
from modularAnalysis import analysis_main, inputMdstList, skimOutputUdst, summaryOfLists
from stdCharged import loadStdCharged
from stdPhotons import stdPhotons
from skimExpertFunctions import setSkimLogging, encodeSkimName

gb2_setuprel = 'release-02-00-00'


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)

stdPhotons('all')
loadStdCharged()

# dark photon skim
from SinglePhotonDark_List import *
darklist = SinglePhotonDarkList()
skimCode = encodeSkimName('SinglePhotonDark')
print("Single photon dark skim:", skimCode)
skimOutputUdst(skimCode, darklist)
summaryOfLists(darklist)

# suppress noisy modules, and then process
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
