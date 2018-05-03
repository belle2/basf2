#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 2 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 03 May 2018
#######################################################

from basf2 import *
from modularAnalysis import *
from stdLightMesons import *
from stdCharged import stdLoosePi
from stdCharged import stdLooseK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
gb2_setuprel = "release-02-00-00"
import sys
import os
import glob

scriptName = sys.argv[0]
skimListName = scriptName[:-19]

skimCode = encodeSkimName(skimListName)
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('default', fileList)

# Load particle lists
stdPhotons('loose')  # gamma:loose
stdLooseK()  # K+/-
stdKshorts()  # K_S0
stdLoosePi()  # pi+/-
stdPi0s('loose')
stdPi0s('all')
loadStdSkimPi0()  # pi0:skim
loadStdLightMesons()  # rho+/-, rho0, K*+/-, K*0, phi, f_0, omega, eta, eta'

# Perform skim
from CharmlessHad2Body_List import *
Had2BodyList = CharmlessHad2BodyB0List() + CharmlessHad2BodyBmList()
skimOutputUdst(skimCode, Had2BodyList)
summaryOfLists(Had2BodyList)

process(analysis_main)
print(statistics)
