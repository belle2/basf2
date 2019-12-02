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

import basf2 as b2
import modularAnalysis as ma
from stdPhotons import stdPhotons
from stdCharged import stdPi
import skimExpertFunctions as expert

# basic setup
gb2_setuprel = 'release-04-00-00'
skimCode = expert.encodeSkimName('BtoXgamma')

path = b2.Path()
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=path)

# import standard lists
stdPhotons('loose', path=path)
stdPhotons('all', path=path)
stdPi('all', path=path)

# call reconstructed lists from scripts/skim/ewp_incl.py
from skim.ewp import B2XgammaList
XgammaList = B2XgammaList(path=path)
expert.skimOutputUdst(skimCode, XgammaList, path=path)
ma.summaryOfLists(XgammaList, path=path)

# process
expert.setSkimLogging(path=path)
b2.process(path=path)

# print out the summary
print(b2.statistics)
