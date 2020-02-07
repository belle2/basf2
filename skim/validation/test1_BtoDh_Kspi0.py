#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoDh_Kspi0.dst.root</input>
    <output>14120300.udst.root</output>
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

skimCode = expert.encodeSkimName('BtoD0h_Kspi0')

fileList = ['../BtoDh_Kspi0.dst.root']

ma.inputMdstList('default', fileList, path=path)

# Load particle lists
stdPi('all', path=path)
stdK('all', path=path)
loadStdSkimPi0(path=path)
stdKshorts(path=path)


# Kspi0 skim
from skim.btocharm import loadD0_Kspi0_loose, BsigToD0hToKspi0List
loadD0_Kspi0_loose(path=path)
BtoDhList = BsigToD0hToKspi0List(path=path)
expert.skimOutputUdst(skimCode, BtoDhList, path=path)
ma.summaryOfLists(BtoDhList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
