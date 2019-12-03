#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# All BtoCharmlessSkims in one _standalone
#
######################################################

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
btocharmlesspath = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=btocharmlesspath)

# Load particle lists
stdPhotons('loose', path=btocharmlesspath)
stdK('loose', path=btocharmlesspath)
stdKshorts(path=btocharmlesspath)
stdPi('loose', path=btocharmlesspath)
stdPi0s('loose', path=btocharmlesspath)
stdPi0s('all', path=btocharmlesspath)
loadStdSkimPi0(path=btocharmlesspath)
loadStdLightMesons(path=btocharmlesspath)

# Import skim decay mode lists and perform skim
from skim.btocharmless import CharmlessHad2BodyB0List, CharmlessHad2BodyBmList
Had2BodyList = CharmlessHad2BodyB0List(path=btocharmlesspath) + CharmlessHad2BodyBmList(path=btocharmlesspath)
expert.add_skim("CharmlessHad2Body", Had2BodyList, path=btocharmlesspath)

from skim.btocharmless import CharmlessHad3BodyB0List, CharmlessHad3BodyBmList
Had3BodyList = CharmlessHad3BodyB0List(path=btocharmlesspath) + CharmlessHad3BodyBmList(path=btocharmlesspath)
expert.add_skim('CharmlessHad3Body', Had3BodyList, path=btocharmlesspath)
expert.setSkimLogging(path=btocharmlesspath)
b2.process(btocharmlesspath)

# print out the summary
print(b2.statistics)
