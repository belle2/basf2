#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoDh_hh.dst.root</input>
    <output>14140100.udst.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "N. Rout"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi
from stdCharged import stdK
from skimExpertFunctions import encodeSkimName, setSkimLogging


hh = Path()

skimCode = encodeSkimName('BtoDh_hh')

fileList = ['BtoDh_hh.dst.root']

inputMdstList('default', fileList, path=hh)

# Load particle lists
stdPi('all', path=hh)
stdK('all', path=hh)


# Hh skim
from skim.btocharm import loadD0bar, BsigToDhTohhList
loadD0bar(path=hh)
BtoDhList = BsigToDhTohhList(path=hh)
skimOutputUdst(skimCode, BtoDhList, path=hh)
summaryOfLists(BtoDhList, path=hh)

# Suppress noisy modules, and then process
setSkimLogging()
process(hh)

# print out the summary
print(statistics)
