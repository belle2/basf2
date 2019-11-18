#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../LeptonicUntagged.dst.root</input>
    <output>../LeptonicUntagged.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
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


path = Path()

fileList = ['../LeptonicUntagged.dst.root']

inputMdstList('default', fileList, path=path)

# Load particle lists
loadStdSkimPi0(path=path)
loadStdSkimPhoton(path=path)
stdPi('loose', path=path)
stdK('loose', path=path)
stdPi('all', path=path)
stdE('all', path=path)
stdMu('all', path=path)
stdPi0s('loose', path=path)  # for stdCharm.py
stdPhotons('loose', path=path)

# Leptonic skim
from skim.leptonic import LeptonicList

lepList = LeptonicList(path=path)
skimOutputUdst('../LeptonicUntagged', lepList, path=path)

summaryOfLists(lepList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
