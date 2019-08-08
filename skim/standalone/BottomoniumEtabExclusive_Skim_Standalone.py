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
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'
import sys
import os
import glob
import argparse

# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

# create a new path
BottomoniumEtabskimpath = Path()

skimCode = encodeSkimName('BottomoniumEtabExclusive')
fileList = get_test_file("mixedBGx1", "MC12")
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
