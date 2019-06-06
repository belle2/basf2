#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 3 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 21 Nov 2018
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
from skimExpertFunctions import encodeSkimName, setSkimLogging

gb2_setuprel = "release-03-00-03"

# Create skim path
charmless3skimpath = Path()

# Retrieve skim code
skimCode = encodeSkimName("CharmlessHad3Body")
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=charmless3skimpath)

# Load particle lists
stdPhotons('loose', path=charmless3skimpath)
stdK('loose', path=charmless3skimpath)
stdKshorts(path=charmless3skimpath)
stdPi('loose', path=charmless3skimpath)
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
