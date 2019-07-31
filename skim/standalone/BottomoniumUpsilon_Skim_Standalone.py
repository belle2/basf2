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
from stdCharged import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'
import sys
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
BottomoniumUpsilonskimpath = Path()

skimCode = encodeSkimName('BottomoniumUpsilon')

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=BottomoniumUpsilonskimpath)

# use standard final state particle lists
stdPi('loose', path=BottomoniumUpsilonskimpath)
stdE('loose', path=BottomoniumUpsilonskimpath)
stdMu('loose', path=BottomoniumUpsilonskimpath)

stdPhotons('loose', path=BottomoniumUpsilonskimpath)

# Bottomonium Skim
from skim.quarkonium import *
YList = UpsilonList(path=BottomoniumUpsilonskimpath)
skimOutputUdst(skimCode, YList, path=BottomoniumUpsilonskimpath)
summaryOfLists(YList, path=BottomoniumUpsilonskimpath)


setSkimLogging(path=BottomoniumUpsilonskimpath)
process(BottomoniumUpsilonskimpath)

# print out the summary
print(statistics)
