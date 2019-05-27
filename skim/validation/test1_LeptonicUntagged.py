#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../LeptonicUntagged.dst.root</input>
    <output>../LeptonicUntagged.udst.root</output>
    <contact>hannah.wakeling@mail.mcgill.ca, philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import stdKshorts
from skim.standardlists.charm import *
from skimExpertFunctions import *


leppath = Path()

skimCode = encodeSkimName('LeptonicUntagged')

fileList = ['../LeptonicUntagged.dst.root']

inputMdstList('default', fileList, path=leppath)

# Load particle lists
loadStdSkimPi0(path=leppath)
loadStdSkimPhoton(path=leppath)
stdPi('loose', path=leppath)
stdK('loose', path=leppath)
stdPi('all', path=leppath)
stdE('all', path=leppath)
stdMu('all', path=leppath)
stdPi0s('loose', path=leppath)  # for stdCharm.py
stdPhotons('loose', path=leppath)

# Leptonic skim
from skim.leptonic import LeptonicList

lepList = LeptonicList(path=leppath)
skimOutputUdst(skimCode, lepList, path=leppath)

summaryOfLists(lepList, path=leppath)

# Suppress noisy modules, and then process
setSkimLogging()
process(leppath)

# print out the summary
print(statistics)
