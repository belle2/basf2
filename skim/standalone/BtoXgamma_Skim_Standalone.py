#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# EWP standalone skim steering
#
# B->Xgamma inclusive skim
#
# Trevor Shillington July 2019
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import stdPi
from skimExpertFunctions import setSkimLogging, encodeSkimName, get_test_file

# basic setup
gb2_setuprel = 'release-04-00-00'
skimCode = encodeSkimName('BtoXgamma')

path = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=path)

# import standard lists
stdPhotons('loose', path=path)
stdPhotons('all', path=path)
stdPi('all', path=path)

# call reconstructed lists from scripts/skim/ewp_incl.py
from skim.ewp import B2XgammaList
XgammaList = B2XgammaList(path=path)
skimOutputUdst(skimCode, XgammaList, path=path)
summaryOfLists(XgammaList, path=path)

# process
setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
