#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 2 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 18 Sep 2018
#######################################################

from basf2 import *
from modularAnalysis import *
from skim.standardlists.lightmesons import *
from stdCharged import stdPi
from stdCharged import stdK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)
gb2_setuprel = "release-02-00-01"
import sys
import os
import glob

# Retrieve skim code
skimCode = encodeSkimName("CharmlessHad2Body")
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)

# Load particle lists
stdPhotons('loose')
stdK('loose')
stdKshorts()
stdPi('loose')
stdPi0s('loose')
stdPi0s('all')
loadStdSkimPi0()
loadStdLightMesons()

# Import skim decay mode lists and perform skim
from skim.btocharmless import CharmlessHad2BodyB0List, CharmlessHad2BodyBmList
Had2BodyList = CharmlessHad2BodyB0List() + CharmlessHad2BodyBmList()
skimOutputUdst(skimCode, Had2BodyList)
summaryOfLists(Had2BodyList)

# Print summary statistics
process(analysis_main)
print(statistics)
