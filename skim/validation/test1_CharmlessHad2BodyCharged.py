#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../CharmlessHad2BodyCharged.dst.root</input>
    <output>../CharmlessHad2BodyCharged.udst.root</output>
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

charmless2chargedpath = b2.Path()

fileList = ['../CharmlessHad2BodyCharged.dst.root']

ma.inputMdstList('default', fileList, path=charmless2chargedpath)

# Load particle lists
stdPhotons('loose', path=charmless2chargedpath)
stdK('loose', path=charmless2chargedpath)
stdKshorts(path=charmless2chargedpath)
stdPi('loose', path=charmless2chargedpath)
stdPi0s('loose', path=charmless2chargedpath)
stdPi0s('all', path=charmless2chargedpath)
loadStdSkimPi0(path=charmless2chargedpath)
loadStdLightMesons(path=charmless2chargedpath)

# Hadronic Bm skim
from skim.btocharmless import CharmlessHad2BodyBmList
Had2BodyList = CharmlessHad2BodyBmList(path=charmless2chargedpath)
expert.skimOutputUdst('../CharmlessHad2BodyCharged.udst.root', Had2BodyList, path=charmless2chargedpath)
ma.summaryOfLists(Had2BodyList, path=charmless2chargedpath)

# Suppress noisy modules, and then process
expert.setSkimLogging(path=charmless2chargedpath)
b2.process(charmless2chargedpath)

# print out the summary
print(b2.statistics)
