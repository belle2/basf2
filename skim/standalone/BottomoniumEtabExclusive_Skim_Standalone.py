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
from stdPhotons import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'
import sys
import os
import glob

# create a new path
BottomoniumEtabskimpath = Path()

skimCode = encodeSkimName('BottomoniumEtabExclusive')
fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]


inputMdstList('MC9', fileList, path=BottomoniumEtabskimpath)


stdPhotons('loose', path=BottomoniumEtabskimpath)
# Bottomonium Skim
from skim.quarkonium import *
EtabList = EtabList(path=BottomoniumEtabskimpath)
skimOutputUdst(skimCode, EtabList, path=BottomoniumEtabskimpath)
summaryOfLists(EtabList, path=BottomoniumEtabskimpath)


setSkimLogging(skim_path=BottomoniumEtabskimpath)
process(BottomoniumEtabskimpath)

# print out the summary
print(statistics)
