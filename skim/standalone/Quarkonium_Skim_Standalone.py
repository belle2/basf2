#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro & Sen Jia, 14/Nov/2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
import sys
import os
import glob

# create a new path
BottomoniumEtabskimpath = Path()

# create a new path
BottomoniumUpsilonskimpath = Path()

fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]


inputMdstList('MC9', fileList, path=BottomoniumEtabskimpath)
inputMdstList('MC9', fileList, path=BottomoniumUpsilonskimpath)

# use standard final state particle lists
stdPi('loose', path=BottomoniumUpsilonskimpath)
stdE('loose', path=BottomoniumUpsilonskimpath)
stdMu('loose', path=BottomoniumUpsilonskimpath)

stdPhotons('loose', path=BottomoniumUpsilonskimpath)
stdPhotons('loose', path=BottomoniumEtabskimpath)

# Bottomonium Etab Skim: 15420100
from skim.quarkonium import EtabList
EtabList = EtabList(path=BottomoniumEtabskimpath)
skimCode1 = encodeSkimName('BottomoniumEtabExclusive')
skimOutputUdst(skimCode1, EtabList, path=BottomoniumEtabskimpath)
summaryOfLists(EtabList, path=BottomoniumEtabskimpath)


# Bottomonium Upsilon Skim: 15440100
from skim.quarkonium import UpsilonList
YList = UpsilonList(path=BottomoniumUpsilonskimpath)
skimCode2 = encodeSkimName('BottomoniumUpsilon')
skimOutputUdst(skimCode2, YList, path=BottomoniumUpsilonskimpath)
summaryOfLists(YList, path=BottomoniumUpsilonskimpath)


setSkimLogging(skim_path=BottomoniumEtabskimpath)
setSkimLogging(skim_path=BottomoniumUpsilonskimpath)
process(BottomoniumEtabskimpath)
process(BottomoniumUpsilonskimpath)

# print out the summary
print(statistics)
