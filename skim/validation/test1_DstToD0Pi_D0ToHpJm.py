#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../DstToD0Pi_D0ToHpJm.dst.root</input>
    <output>17240100.udst.root</output>
    <contact>gonggd@mail.ustc.edu.cn</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "G. GONG"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi
from stdCharged import stdK
from skimExpertFunctions import encodeSkimName, setSkimLogging


c2bhdpath = Path()

skimCode = encodeSkimName('DstToD0Pi_D0ToHpJm')

fileList = ['../DstToD0Pi_D0ToHpJm.dst.root']

inputMdstList('default', fileList, path=c2bhdpath)

# Load particle lists
stdPi('loose', path=c2bhdpath)
stdK('loose', path=c2bhdpath)
stdPi('all', path=c2bhdpath)
stdK('all', path=c2bhdpath)

# Charm 2 Body Hadronic skim
from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(path=c2bhdpath)
skimOutputUdst(skimCode, DstToD0PiD0ToHpJmList, path=c2bhdpath)
summaryOfLists(DstToD0PiD0ToHpJmList, path=c2bhdpath)

# Suppress noisy modules, and then process
setSkimLogging(path=c2bhdpath)
process(c2bhdpath)

# print out the summary
print(statistics)
