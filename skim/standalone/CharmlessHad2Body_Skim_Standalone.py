#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 2 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 21 Nov 2018
#######################################################

import basf2 as b2
import modularAnalysis as ma
from skim.standardlists.lightmesons import loadStdLightMesons
from stdCharged import stdK, stdPi
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
import skimExpertFunctions as expert
gb2_setuprel = "release-04-00-00"

# Create skim path
charmless2skimpath = b2.Path()

# Retrieve skim code
skimCode = expert.encodeSkimName("CharmlessHad2Body")
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=charmless2skimpath)

# Load particle lists
stdPhotons('loose', path=charmless2skimpath)
stdK('loose', path=charmless2skimpath)
stdKshorts(path=charmless2skimpath)
stdPi('loose', path=charmless2skimpath)
stdPi0s('loose', path=charmless2skimpath)
stdPi0s('all', path=charmless2skimpath)
loadStdSkimPi0(path=charmless2skimpath)
loadStdLightMesons(path=charmless2skimpath)

# Import skim decay mode lists and perform skim
from skim.btocharmless import CharmlessHad2BodyB0List, CharmlessHad2BodyBmList
Had2BodyList = CharmlessHad2BodyB0List(path=charmless2skimpath) + CharmlessHad2BodyBmList(path=charmless2skimpath)
expert.skimOutputUdst(skimCode, Had2BodyList, path=charmless2skimpath)
ma.summaryOfLists(Had2BodyList, path=charmless2skimpath)

expert.setSkimLogging(path=charmless2skimpath)
b2.process(charmless2skimpath)

# Print summary statistics
print(b2.statistics)
