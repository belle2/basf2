#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skims for electron-muon-based dark sector searches
# Giacomo De Pietro 2019 (giacomo.depietro@roma3.infn.it)
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
from skim.dark import ElectronMuonForDarkSearchesList as ElectronMuonList

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'
skimCode = encodeSkimName('ElectronMuonForDarkSearches')

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

dimuon_path = Path()

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=dimuon_path)

stdMu('all', path=dimuon_path)

dimuon_list = ElectronMuonList(path=dimuon_path)
skimOutputUdst(skimCode, dimuon_list, path=dimuon_path)
summaryOfLists(dimuon_list, path=dimuon_path)

setSkimLogging(path=dimuon_path)
process(dimuon_path)

print(statistics)
