#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# EWP standalone skim steering
#
# B->Xll (LFV modes only) inclusive skim
#
# Trevor Shillington July 2019
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import stdE, stdMu, stdPi
from skimExpertFunctions import setSkimLogging, encodeSkimName, get_test_file

# basic setup
gb2_setuprel = 'release-04-00-00'
skimCode = encodeSkimName('BtoXll_LFV')

path = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=path)

# import standard lists
stdE('loose', path=path)
stdMu('loose', path=path)
stdPi('all', path=path)
stdPhotons('all', path=path)

# call reconstructed lists from scripts/skim/ewp.py
from skim.ewp import B2XllListLFV
XllList = B2XllListLFV(path=path)
skimOutputUdst(skimCode, XllList, path=path)
summaryOfLists(XllList, path=path)

# process
setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
