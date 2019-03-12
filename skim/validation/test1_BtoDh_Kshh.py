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


kshh = Path()

skimCode = encodeSkimName('BtoDh_Kshh')

fileList = ['BtoDh_Kshh.dst.root']

inputMdstList('default', fileList, path=kshh)

# Load particle lists
stdPi('all', path=kshh)
stdK('all', path=kshh)
stdKshorts(path=kshh)


# Kshh skim
from skim.btocharm import loadDkshh, BsigToDhToKshhList
loadDkshh(path=kshh)
BtoDhList = BsigToDhToKshhList(path=kshh)
skimOutputUdst(skimCode, BtoDhList, path=kshh)
summaryOfLists(BtoDhList, path=kshh)

# Suppress noisy modules, and then process
setSkimLogging()
process(kshh)

# print out the summary
print(statistics)
