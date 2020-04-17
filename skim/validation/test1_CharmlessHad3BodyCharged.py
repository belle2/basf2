#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../CharmlessHad3BodyCharged.dst.root</input>
    <output>../CharmlessHad3BodyCharged.udst.root</output>
    <contact>khsmith@student.unimelb.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "K. Smith"

import basf2 as b2
import modularAnalysis as ma
from skim.standardlists.lightmesons import loadStdLightMesons
from stdCharged import stdK, stdPi
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
import skimExpertFunctions as expert

charmless3chargedpath = b2.Path()

fileList = ['../CharmlessHad3BodyCharged.dst.root']

ma.inputMdstList('default', fileList, path=charmless3chargedpath)

# Load particle lists
stdPhotons('loose', path=charmless3chargedpath)
stdK('loose', path=charmless3chargedpath)
stdKshorts(path=charmless3chargedpath)
stdPi('loose', path=charmless3chargedpath)
stdPi0s('eff40_Jan2020', path=charmless3chargedpath)
stdPi0s('all', path=charmless3chargedpath)
loadStdSkimPi0(path=charmless3chargedpath)
loadStdLightMesons(path=charmless3chargedpath)

# Hadronic Bm skim
from skim.btocharmless import CharmlessHad3BodyBmList
Had3BodyList = CharmlessHad3BodyBmList(path=charmless3chargedpath)
expert.skimOutputUdst('../CharmlessHad3BodyCharged.udst.root', Had3BodyList, path=charmless3chargedpath)
ma.summaryOfLists(Had3BodyList, path=charmless3chargedpath)

# Suppress noisy modules, and then process
expert.setSkimLogging(path=charmless3chargedpath)
b2.process(charmless3chargedpath)

# print out the summary
print(b2.statistics)
