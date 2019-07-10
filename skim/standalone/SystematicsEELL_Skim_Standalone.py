#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for four-lepton events
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

skimCode = encodeSkimName('SystematicsEELL')
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

skimpath = Path()


fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=skimpath)

stdE('all', path=skimpath)
stdMu('all', path=skimpath)

from skim.systematics import *
SysList = EELLList(skimpath)
skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)
