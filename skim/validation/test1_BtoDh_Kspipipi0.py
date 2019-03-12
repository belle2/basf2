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
__author__ = "N. Rout"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi
from stdCharged import stdK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from skimExpertFunctions import encodeSkimName, setSkimLogging


kspipipi0 = Path()

skimCode = encodeSkimName('BtoDh_Kspipipi0')

fileList = ['BtoDh_Kspipipi0.dst.root']

inputMdstList('default', fileList, path=kspipipi0)

# Load particle lists
stdPi('all', path=kspipipi0)
stdK('all', path=kspipipi0)
loadStdSkimPi0(path=kspipipi0)
stdKshorts(path=kspipipi0)


# Kspipipi0 skim
from skim.btocharm import loadDkspipipi0, BsigToDhToKspipipi0List
loadDkspipipi0(path=kspipipi0)
BtoDhList = BsigToDhToKspipipi0List(path=kspipipi0)
skimOutputUdst(skimCode, BtoDhList, path=kspipipi0)
summaryOfLists(BtoDhList, path=kspipipi0)

# Suppress noisy modules, and then process
setSkimLogging()
process(kspipipi0)

# print out the summary
print(statistics)
