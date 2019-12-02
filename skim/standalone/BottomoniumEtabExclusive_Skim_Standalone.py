#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro, 25/Jul/2016
#
######################################################
import basf2 as b2
import modularAnalysis as ma
from stdPhotons import stdPhotons
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

# create a new path
BottomoniumEtabskimpath = b2.Path()

skimCode = expert.encodeSkimName('BottomoniumEtabExclusive')
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=BottomoniumEtabskimpath)


stdPhotons('loose', path=BottomoniumEtabskimpath)
# Bottomonium Skim
from skim.quarkonium import *
EtabList = EtabList(path=BottomoniumEtabskimpath)
expert.skimOutputUdst(skimCode, EtabList, path=BottomoniumEtabskimpath)
ma.summaryOfLists(EtabList, path=BottomoniumEtabskimpath)


expert.setSkimLogging(path=BottomoniumEtabskimpath)
b2.process(BottomoniumEtabskimpath)

# print out the summary
print(b2.statistics)
