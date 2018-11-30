#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP standalone skim steering
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from stdPhotons import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'
import sys
import os
import glob


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList)
stdPi0s('loose')
stdPhotons('tight')  # also builds loose list
loadStdSkimPhoton()
loadStdSkimPi0()
stdK('loose')
stdPi('loose')
stdK('95eff')
stdPi('95eff')
stdKshorts()
stdPhotons('loose')
stdE('95eff')
stdMu('95eff')

loadStdLightMesons()


# EWP Skim
from skim.ewp import B2XllList
XllList = B2XllList()
skimCode1 = encodeSkimName('BtoXll')
skimOutputUdst(skimCode1, XllList)
summaryOfLists(XllList)


# EWP Skim
from skim.ewp import B2XgammaList
XgammaList = B2XgammaList()
skimCode2 = encodeSkimName('BtoXgamma')
skimOutputUdst(skimCode2, XgammaList)
summaryOfLists(XgammaList)


setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
