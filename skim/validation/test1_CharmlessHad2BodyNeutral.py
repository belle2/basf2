#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../CharmlessHad2BodyNeutral.dst.root</input>
    <output>../CharmlessHad2BodyNeutral.udst.root</output>
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

charmless2neutralpath = b2.Path()

fileList = ['../CharmlessHad2BodyNeutral.dst.root']

ma.inputMdstList('default', fileList, path=charmless2neutralpath)

# Load particle lists
stdPhotons('loose', path=charmless2neutralpath)
stdK('loose', path=charmless2neutralpath)
stdKshorts(path=charmless2neutralpath)
stdPi('loose', path=charmless2neutralpath)
stdPi0s('eff40_Jan2020', path=charmless2neutralpath)
stdPi0s('all', path=charmless2neutralpath)
loadStdSkimPi0(path=charmless2neutralpath)
loadStdLightMesons(path=charmless2neutralpath)

# Hadronic B0 skim
from skim.btocharmless import CharmlessHad2BodyB0List
Had2BodyList = CharmlessHad2BodyB0List(path=charmless2neutralpath)
expert.skimOutputUdst('../CharmlessHad2BodyNeutral.udst.root', Had2BodyList, path=charmless2neutralpath)
ma.summaryOfLists(Had2BodyList, path=charmless2neutralpath)

# Suppress noisy modules, and then process
expert.setSkimLogging(path=charmless2neutralpath)
b2.process(charmless2neutralpath)

# print out the summary
print(b2.statistics)
