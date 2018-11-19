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
from stdCharged import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'
import sys
import os
import glob

fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]


inputMdstList('MC9', fileList)


stdPhotons('loose')

loadStdCharged()


def add_skim(label, lists):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.
    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists)
    summaryOfLists(lists)


# Bottomonium Etab Skim: 15420100
from skim.quarkonium import EtabList
add_skim('BottomoniumEtabExclusive', EtabList())


# Bottomonium Upsilon Skim: 15440100
from skim.quarkonium import UpsilonList
add_skim('BottomoniumUpsilon', UpsilonList())


# ISR cc skim
from skim.quarkonium import ISRpipiccList
add_skim('ISRpipicc', ISRpipiccList())

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
