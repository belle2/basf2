#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# EWP standalone skim steering
#
# B->Xll (no LFV modes) inclusive skim
#
# Trevor Shillington July 2019
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import stdE, stdMu, stdPi
from skimExpertFunctions import setSkimLogging, encodeSkimName, get_test_file

gb2_setuprel = 'release-04-00-00'
skimCode = encodeSkimName('BtoXll')

path = Path()
fileList = get_test_file("MC12_mixedBGx1")
inputMdstList('default', fileList, path=path)

# import standard lists
stdE('loose', path=path)
stdMu('loose', path=path)
stdPi('all', path=path)
stdPhotons('all', path=path)

# call reconstructed lists from scripts/skim/ewp.py
from skim.ewp import B2XllList
XllList = B2XllList(path=path)
skimOutputUdst(skimCode, XllList, path=path)
summaryOfLists(XllList, path=path)

# process
setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
