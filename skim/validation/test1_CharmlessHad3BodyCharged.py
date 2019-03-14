#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../CharmlessHad3BodyCharged.dst.root</input>
    <output>CharmlessHad3BodyCharged.udst.root</output>
    <contact>khsmith@student.unimelb.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
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

charmless3chargedpath = Path()

fileList = ['../CharmlessHad3BodyCharged.dst.root']

inputMdstList('MC9', fileList, path=charmless3chargedpath)

# Load particle lists
stdPhotons('loose', path=charmless3chargedpath)
stdK('loose', path=charmless3chargedpath)
stdKshorts(path=charmless3chargedpath)
stdPi('loose', path=charmless3chargedpath)
stdPi0s('loose', path=charmless3chargedpath)
stdPi0s('all', path=charmless3chargedpath)
loadStdSkimPi0(path=charmless3chargedpath)
loadStdLightMesons(path=charmless3chargedpath)

# Hadronic B0 skim
from skim.btocharmless import *
Had3BodyList = CharmlessHad3BodyB0List(path=charmless3chargedpath) + CharmlessHad3BodyBmList(path=charmless3chargedpath)
skimOutputUdst('CharmlessHad3BodyCharged.udst.root', Had3BodyList, path=charmless3chargedpath)
summaryOfLists(Had3BodyList, path=charmless3chargedpath)

# Suppress noisy modules, and then process
setSkimLogging(path=charmless3chargedpath)
process(charmless3chargedpath)

# print out the summary
print(statistics)
