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


path = Path()

skimCode = encodeSkimName('BtoDh_hh')

fileList = ['../BtoDh_hh.dst.root']

inputMdstList('default', fileList, path=path)

# Load particle lists
stdPi('all', path=path)
stdK('all', path=path)


# Hh skim
from skim.btocharm import loadD0bar, BsigToDhTohhList
loadD0bar(path=path)
BtoDhList = BsigToDhTohhList(path=path)
skimOutputUdst(skimCode, BtoDhList, path=path)
summaryOfLists(BtoDhList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
