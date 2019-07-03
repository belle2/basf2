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
__author__ = "N. Rout"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi
from stdCharged import stdK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from skimExpertFunctions import encodeSkimName, setSkimLogging


kspi0 = Path()

skimCode = encodeSkimName('BtoDh_Kspi0')

fileList = ['../BtoDh_Kspi0.dst.root']

inputMdstList('default', fileList, path=kspi0)

# Load particle lists
stdPi('all', path=kspi0)
stdK('all', path=kspi0)
loadStdSkimPi0(path=kspi0)
stdKshorts(path=kspi0)


# Kspi0 skim
from skim.btocharm import loadDkspi0, BsigToDhToKspi0List
loadDkspi0(path=kspi0)
BtoDhList = BsigToDhToKspi0List(path=kspi0)
skimOutputUdst(skimCode, BtoDhList, path=kspi0)
summaryOfLists(BtoDhList, path=kspi0)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(kspi0)

# print out the summary
print(statistics)
