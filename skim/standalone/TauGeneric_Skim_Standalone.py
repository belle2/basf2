#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau generic skims
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
import argparse
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

skimCode = encodeSkimName('TauGeneric')

# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

taugenericskim = Path()

fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# Tau Skim
from skim.taupair import *
tauList = TauList(path=taugenericskim)

skimOutputUdst(skimCode, tauList, path=taugenericskim)
summaryOfLists(tauList, path=taugenericskim)

setSkimLogging(path=taugenericskim)
process(taugenericskim)

# print out the summary
print(statistics)
