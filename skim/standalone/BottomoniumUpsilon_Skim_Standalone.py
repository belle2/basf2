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
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'
import sys
import os
import glob
skimCode = encodeSkimName('BottomoniumUpsilon')
fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]

inputMdstList('MC9', fileList)
stdPhotons('loose')

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList()
skimOutputUdst(skimCode, YList)
summaryOfLists(YList)


setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
