# !/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B0 -> pi0 pi0
#
# K. Smith
#
#######################################################

from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
from stdPi0s import *

from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
set_log_level(LogLevel.INFO)

gb2_setuprel = "release-03-02-00"

import os
import sys
import glob
import argparse
skimCode = encodeSkimName('BtoPi0Pi0')

# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

path = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=path)

# load particle lists
loadStdSkimPi0(path=path)

# Had Skim
from skim.btocharmless import BtoPi0Pi0List
Pi0Pi0List = BtoPi0Pi0List(path=path)
skimOutputUdst(skimCode, Pi0Pi0List, path=path)

summaryOfLists(Pi0Pi0List, path=path)


setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
