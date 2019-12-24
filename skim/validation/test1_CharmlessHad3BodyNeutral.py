#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../CharmlessHad3BodyNeutral.dst.root</input>
    <output>../CharmlessHad3BodyNeutral.udst.root</output>
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

charmless3neutralpath = b2.Path()

fileList = ['../CharmlessHad3BodyNeutral.dst.root']

ma.inputMdstList('default', fileList, path=charmless3neutralpath)

# Load particle lists
stdPhotons('loose', path=charmless3neutralpath)
stdK('loose', path=charmless3neutralpath)
stdKshorts(path=charmless3neutralpath)
stdPi('loose', path=charmless3neutralpath)
stdPi0s('loose', path=charmless3neutralpath)
stdPi0s('all', path=charmless3neutralpath)
loadStdSkimPi0(path=charmless3neutralpath)
loadStdLightMesons(path=charmless3neutralpath)

# Hadronic B0 skim
from skim.btocharmless import CharmlessHad3BodyB0List
Had3BodyList = CharmlessHad3BodyB0List(path=charmless3neutralpath)
expert.skimOutputUdst('../CharmlessHad3BodyNeutral.udst.root', Had3BodyList, path=charmless3neutralpath)
ma.summaryOfLists(Had3BodyList, path=charmless3neutralpath)

# Suppress noisy modules, and then process
expert.setSkimLogging(path=charmless3neutralpath)
b2.process(charmless3neutralpath)

# print out the summary
print(b2.statistics)
