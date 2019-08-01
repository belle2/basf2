#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# Charm skims
# P. Urquijo, 6/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu, stdPi
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")


gb2_setuprel = 'release-03-02-02'
set_log_level(LogLevel.INFO)
import os
import sys
import glob

# skimCode = encodeSkimName('CharmRare')
skimCode = "CharmRare"

crpath = Path()

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=crpath)


loadStdSkimPi0(path=crpath)
loadStdSkimPhoton(path=crpath)
stdMu('loose', path=crpath)
stdE('loose', path=crpath)
stdPi('loose', path=crpath)

from skim.charm import CharmRare
CharmRareList = CharmRare(crpath)
skimOutputUdst(skimCode, CharmRareList, path=crpath)
summaryOfLists(CharmRareList, path=crpath)

setSkimLogging(path=crpath)
process(crpath)

print(statistics)
