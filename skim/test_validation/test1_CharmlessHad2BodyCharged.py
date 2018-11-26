#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__author__ = "K. Smith"


from basf2 import *
from modularAnalysis import *
from skim.standardlists.lightmesons import loadStdLightMesons
from stdCharged import stdPi
from stdCharged import stdK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
from skimExpertFunctions import setSkimLogging

charmless2chargedpath = Path()

fileList = ['../CharmlessHad2BodyCharged.dst.root']

inputMdstList('MC9', fileList, path=charmless2chargedpath)

# Load particle lists
stdPhotons('loose', path=charmless2chargedpath)
stdK('loose', path=charmless2chargedpath)
stdKshorts(path=charmless2chargedpath)
stdPi('loose', path=charmless2chargedpath)
stdPi0s('loose', path=charmless2chargedpath)
stdPi0s('all', path=charmless2chargedpath)
loadStdSkimPi0(path=charmless2chargedpath)
loadStdLightMesons(path=charmless2chargedpath)

# Hadronic B0 skim
from skim.btocharmless import *
Had2BodyList = CharmlessHad2BodyB0List(path=charmless2chargedpath) + CharmlessHad2BodyBmList(path=charmless2chargedpath)
skimOutputUdst('CharmlessHad2BodyCharged.udst.root', Had2BodyList, path=charmless2chargedpath)
summaryOfLists(Had2BodyList, path=charmless2chargedpath)

# Suppress noisy modules, and then process
setSkimLogging(skim_path=charmless2chargedpath)
process(charmless2chargedpath)

# print out the summary
print(statistics)
