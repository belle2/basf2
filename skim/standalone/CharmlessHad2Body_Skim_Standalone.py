#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 2 Body skims
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
charmless2skimpath = Path()

# Retrieve skim code
skimCode = encodeSkimName("CharmlessHad2Body")
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=charmless2skimpath)

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
skimOutputUdst(skimCode, Had2BodyList, path=charmless2skimpath)
summaryOfLists(Had2BodyList, path=charmless2skimpath)

setSkimLogging(path=charmless2skimpath)
process(charmless2skimpath)

# Print summary statistics
print(statistics)
