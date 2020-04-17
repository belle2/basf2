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

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s, loadStdSkimPi0
import skimExpertFunctions as expert


path = b2.Path()

fileList = ['../LeptonicUntagged.dst.root']

ma.inputMdstList('default', fileList, path=path)

# Load particle lists
loadStdSkimPi0(path=path)
loadStdSkimPhoton(path=path)
stdPi('loose', path=path)
stdK('loose', path=path)
stdPi('all', path=path)
stdE('all', path=path)
stdMu('all', path=path)
stdPi0s('eff40_Jan2020', path=path)  # for stdCharm.py
stdPhotons('loose', path=path)

# Leptonic skim
from skim.leptonic import LeptonicList

lepList = LeptonicList(path=path)
expert.skimOutputUdst('../LeptonicUntagged', lepList, path=path)

ma.summaryOfLists(lepList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
