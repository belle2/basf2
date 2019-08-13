#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""FEI Semi-leptonic B0 tag skim standalone for generic analysis in th
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180400
    fei training: MC12 based, release-03-02-00 'FEIv4_2019_MC12_release_03_01_01'
 """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]

import sys
import glob
import os.path
import argparse

from basf2 import *
from modularAnalysis import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'
skimCode = encodeSkimName('feiSLBplus')
fileList = get_test_file("mixedBGx1", "MC12")

# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

path = create_path()

inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforBplusSL(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B+ tag cuts
BtagList = BplusSL(path)
skimOutputUdst(skimCode, BtagList, path=path)
summaryOfLists(BtagList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
