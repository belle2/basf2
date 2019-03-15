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
gb2_setuprel = 'release-03-00-03'
import sys
import os
import glob

# create a new path
BottomoniumEtabskimpath = Path()

skimCode = encodeSkimName('BottomoniumEtabExclusive')
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=BottomoniumEtabskimpath)


stdPhotons('loose', path=BottomoniumEtabskimpath)
# Bottomonium Skim
from skim.quarkonium import *
EtabList = EtabList(path=BottomoniumEtabskimpath)
skimOutputUdst(skimCode, EtabList, path=BottomoniumEtabskimpath)
summaryOfLists(EtabList, path=BottomoniumEtabskimpath)


setSkimLogging(path=BottomoniumEtabskimpath)
process(BottomoniumEtabskimpath)

# print out the summary
print(statistics)
