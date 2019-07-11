#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP standalone skim steering
#
# B->Xll (LFV modes only) inclusive skim
#
# T.R. Shillington July 2019
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import stdE, stdMu, stdPi
from skimExpertFunctions import setSkimLogging, encodeSkimName, get_test_file
import argparse

# Use argparse to allow the optional --data argument, used only when run on data
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

# basic setup
gb2_setuprel = 'release-03-02-02'
skimCode = encodeSkimName('BtoXll')

path = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=path)

# import standard lists
stdE('loose', path=path)
stdMu('loose', path=path)
stdPi('all', path=path)
stdPhotons('all', path=path)

# call reconstructed lists from scripts/skim/ewp_incl.py
from skim.ewp import B2XllListLFV
XllList = B2XllListLFV(path=path)
skimOutputUdst(skimCode, XllList, path=path)
summaryOfLists(XllList, path=path)

# process
setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
