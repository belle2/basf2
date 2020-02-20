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

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
import skimExpertFunctions as expert


c2bhdpath = b2.Path()

skimCode = expert.encodeSkimName('DstToD0Pi_D0ToHpJm')

fileList = ['../DstToD0Pi_D0ToHpJm.dst.root']

ma.inputMdstList('default', fileList, path=c2bhdpath)

# Load particle lists
stdPi('loose', path=c2bhdpath)
stdK('loose', path=c2bhdpath)
stdPi('all', path=c2bhdpath)
stdK('all', path=c2bhdpath)

# Charm 2 Body Hadronic skim
from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(path=c2bhdpath)
expert.skimOutputUdst(skimCode, DstToD0PiD0ToHpJmList, path=c2bhdpath)
ma.summaryOfLists(DstToD0PiD0ToHpJmList, path=c2bhdpath)

# Suppress noisy modules, and then process
expert.setSkimLogging(path=c2bhdpath)
b2.process(c2bhdpath)

# print out the summary
print(b2.statistics)
