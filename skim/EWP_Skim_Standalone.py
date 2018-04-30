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
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *
from stdLightMesons import *
from stdPhotons import *
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-00'
import sys
import os
import glob


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)
stdPi0s('loose')
# stdPhotons('loose')
stdPhotons('tight')  # also builds loose list
loadStdSkimPhoton()
loadStdSkimPi0()
loadStdCharged()
stdK('95eff')
stdPi('95eff')
stdKshorts()
loadStdLightMesons()
stdPhotons('loose')
loadStdCharged()
stdE('95eff')
stdMu('95eff')
stdMu('90eff')
stdKshorts()

# EWP Skim
from EWP_List import *
XllList = B2XllList()
outputLFN1 = getOutputLFN('BtoXll')
skimOutputUdst(outputLFN1, XllList)
summaryOfLists(XllList)


# EWP Skim
from EWP_List import *
XgammaList = B2XgammaList()
outputLFN2 = getOutputLFN('BtoXgamma')
skimOutputUdst(outputLFN2, XgammaList)
summaryOfLists(XgammaList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)

process(analysis_main)

# print out the summary
print(statistics)
