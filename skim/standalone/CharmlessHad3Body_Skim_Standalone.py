#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 3 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 18 Nov 2018
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

# Create skim path
charmless3skimpath = Path()

# Retrieve skim code
skimCode = encodeSkimName("CharmlessHad3Body")
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=charmless3skimpath)

# Load particle lists
stdPhotons('loose', path=charmless3skimpath)
stdLooseK(path=charmless3skimpath)
stdKshorts(path=charmless3skimpath)
stdLoosePi(path=charmless3skimpath)
stdPi0s('loose', path=charmless3skimpath)
stdPi0s('all', path=charmless3skimpath)
loadStdSkimPi0(path=charmless3skimpath)
loadStdLightMesons(path=charmless3skimpath)

# Import skim decay mode lists and perform skim
from skim.btocharmless import CharmlessHad3BodyB0List, CharmlessHad3BodyBmList
Had3BodyList = CharmlessHad3BodyB0List(path=charmless3skimpath) + CharmlessHad3BodyBmList(path=charmless3skimpath)
skimOutputUdst(skimCode, Had3BodyList, path=charmless3skimpath)
summaryOfLists(Had3BodyList, path=charmless3skimpath)

setSkimLogging(path=charmless3skimpath)
process(charmless3skimpath)

# Print summary statistics
print(statistics)
