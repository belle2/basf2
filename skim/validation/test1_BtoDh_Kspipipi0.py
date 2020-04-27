#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoDh_Kspipipi0.dst.root</input>
    <output>14120400.udst.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = [
    "N Dash",
    "N. Rout"
]
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPi0s import loadStdSkimPi0
from stdV0s import stdKshorts
import skimExpertFunctions as expert


path = b2.Path()

skimCode = expert.encodeSkimName('BtoD0h_Kspipipi0')

fileList = ['../BtoDh_Kspipipi0.dst.root']

ma.inputMdstList('default', fileList, path=path)

# Load particle lists
stdPi('all', path=path)
stdK('all', path=path)
loadStdSkimPi0(path=path)
stdKshorts(path=path)


# Kspipipi0 skim
from skim.btocharm import BsigToD0hToKspipipi0List
from skim.standardlists.charm import loadD0_Kspipipi0

loadD0_Kspipipi0(path=path)
BtoDhList = BsigToD0hToKspipipi0List(path=path)
expert.skimOutputUdst(skimCode, BtoDhList, path=path)
ma.summaryOfLists(BtoDhList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
