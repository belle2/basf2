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

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
import skimExpertFunctions as expert


path = b2.Path()

skimCode = expert.encodeSkimName('BtoDh_hh')

fileList = ['../BtoDh_hh.dst.root']

ma.inputMdstList('default', fileList, path=path)

# Load particle lists
stdPi('all', path=path)
stdK('all', path=path)


# Hh skim
from skim.btocharm import loadD0bar, BsigToDhTohhList
loadD0bar(path=path)
BtoDhList = BsigToDhTohhList(path=path)
expert.skimOutputUdst(skimCode, BtoDhList, path=path)
ma.summaryOfLists(BtoDhList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
