#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoDh_Kshh.dst.root</input>
    <output>14140200.udst.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = [
    "N. Dash",
    "N. Rout"
]

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdV0s import stdKshorts
import skimExpertFunctions as expert


path = b2.Path()

skimCode = expert.encodeSkimName('BtoD0h_Kshh')

fileList = ['../BtoDh_Kshh.dst.root']

ma.inputMdstList('default', fileList, path=path)

# Load particle lists
stdPi('all', path=path)
stdK('all', path=path)
stdKshorts(path=path)


# Kshh skim
from skim.btocharm import BsigToD0hToKshhList
from skim.standardlists.charm import loadD0_Kshh_loose

loadD0_Kshh_loose(path=path)
BtoDhList = BsigToD0hToKshhList(path=path)
expert.skimOutputUdst(skimCode, BtoDhList, path=path)
ma.summaryOfLists(BtoDhList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
