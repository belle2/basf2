#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skims for LFV Z'
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

skimCode = encodeSkimName('LFVZpInvisible')
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

lfvzppath = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=lfvzppath)
stdPi('loose', path=lfvzppath)
stdK('loose', path=lfvzppath)
stdE('loose', path=lfvzppath)
stdMu('loose', path=lfvzppath)
stdPi('all', path=lfvzppath)
stdK('all', path=lfvzppath)
stdE('all', path=lfvzppath)
stdMu('all', path=lfvzppath)

from skim.dark import LFVZpInvisibleList
SysList = LFVZpInvisibleList(path=lfvzppath)
skimOutputUdst(skimCode, SysList, path=lfvzppath)
summaryOfLists(SysList, path=lfvzppath)

setSkimLogging(path=lfvzppath)
process(lfvzppath)

print(statistics)
