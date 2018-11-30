#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../CharmlessHad2BodyNeutral.dst.root</input>
    <output>CharmlessHad2BodyNeutral.udst.root</output>
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

charmless2neutralpath = Path()

fileList = ['../CharmlessHad2BodyNeutral.dst.root']

inputMdstList('MC9', fileList, path=charmless2neutralpath)

# Load particle lists
stdPhotons('loose', path=charmless2neutralpath)
stdK('loose', path=charmless2neutralpath)
stdKshorts(path=charmless2neutralpath)
stdPi('loose', path=charmless2neutralpath)
stdPi0s('loose', path=charmless2neutralpath)
stdPi0s('all', path=charmless2neutralpath)
loadStdSkimPi0(path=charmless2neutralpath)
loadStdLightMesons(path=charmless2neutralpath)

# Hadronic B0 skim
from skim.btocharmless import *
Had2BodyList = CharmlessHad2BodyB0List(path=charmless2neutralpath) + CharmlessHad2BodyBmList(path=charmless2neutralpath)
skimOutputUdst('CharmlessHad2BodyNeutral.udst.root', Had2BodyList, path=charmless2neutralpath)
summaryOfLists(Had2BodyList, path=charmless2neutralpath)

# Suppress noisy modules, and then process
setSkimLogging(skim_path=charmless2neutralpath)
process(charmless2neutralpath)

# print out the summary
print(statistics)
