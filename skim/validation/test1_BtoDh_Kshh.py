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
__author__ = "N. Rout"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi
from stdCharged import stdK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from skimExpertFunctions import encodeSkimName, setSkimLogging


path = Path()

skimCode = encodeSkimName('BtoDh_Kshh')

fileList = ['../BtoDh_Kshh.dst.root']

inputMdstList('default', fileList, path=path)

# Load particle lists
stdPi('all', path=path)
stdK('all', path=path)
stdKshorts(path=path)


# Kshh skim
from skim.btocharm import loadDkshh, BsigToDhToKshhList
loadDkshh(path=path)
BtoDhList = BsigToDhToKshhList(path=path)
skimOutputUdst(skimCode, BtoDhList, path=path)
summaryOfLists(BtoDhList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
