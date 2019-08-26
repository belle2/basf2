#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skims for dielectron-based dark sector searches
# Giacomo De Pietro 2019 (giacomo.depietro@roma3.infn.it)
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
from skim.dark import DielectronForDarkSearchesList as DielectronList

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'
skimCode = encodeSkimName('DielectronForDarkSearches')

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
    use_central_database("data_reprocessing_proc9")

dielectron_path = Path()

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=dielectron_path)

stdE('all', path=dielectron_path)

dielectron_list = DielectronList(path=dielectron_path)
skimOutputUdst(skimCode, dielectron_list, path=dielectron_path)
summaryOfLists(dielectron_list, path=dielectron_path)

setSkimLogging(path=dielectron_path)
process(dielectron_path)

print(statistics)
