#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(*) h(*) skims
# P. Urquijo, 6/Jan/2015
#
######################################################
from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdV0s import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
import argparse

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

path = Path()

skimCode = encodeSkimName('BtoDh_Kshh')


fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=path)


stdPi('all', path=path)
stdK('all', path=path)
stdKshorts(path=path)

# B- to D(->Kshh)h- Skim
from skim.btocharm import loadDkshh, BsigToDhToKshhList
loadDkshh(path=path)
BtoDhList = BsigToDhToKshhList(path=path)
skimOutputUdst(skimCode, BtoDhList, path=path)
summaryOfLists(BtoDhList, path=path)


setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
