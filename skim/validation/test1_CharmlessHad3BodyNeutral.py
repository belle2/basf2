#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../CharmlessHad3BodyNeutral.dst.root</input>
    <output>CharmlessHad3BodyNeutral.udst.root</output>
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

charmless3neutralpath = Path()

fileList = ['../CharmlessHad3BodyNeutral.dst.root']

inputMdstList('MC9', fileList, path=charmless3neutralpath)

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
from skim.btocharmless import *
Had3BodyList = CharmlessHad3BodyB0List(path=charmless3neutralpath) + CharmlessHad3BodyBmList(path=charmless3neutralpath)
skimOutputUdst('CharmlessHad3BodyNeutral.udst.root', Had3BodyList, path=charmless3neutralpath)
summaryOfLists(Had3BodyList, path=charmless3neutralpath)

# Suppress noisy modules, and then process
setSkimLogging(skim_path=charmless3neutralpath)
process(charmless3neutralpath)

# print out the summary
print(statistics)
