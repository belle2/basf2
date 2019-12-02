#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 3 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 21 Nov 2018
#######################################################

import basf2 as b2
import modularAnalysis as ma
from skim.standardlists.lightmesons import loadStdLightMesons
from stdCharged import stdK, stdPi
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
import skimExpertFunctions as expert
gb2_setuprel = "release-04-00-00"

# Create skim path
charmless3skimpath = b2.Path()

# Retrieve skim code
skimCode = expert.encodeSkimName("CharmlessHad3Body")
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=charmless3skimpath)

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
expert.skimOutputUdst(skimCode, Had3BodyList, path=charmless3skimpath)
ma.summaryOfLists(Had3BodyList, path=charmless3skimpath)

expert.setSkimLogging(path=charmless3skimpath)
b2.process(charmless3skimpath)

# Print summary statistics
print(b2.statistics)
