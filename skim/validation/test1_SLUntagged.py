#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../SLUntagged.dst.root</input>
    <output>../SLUntagged.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
import skimExpertFunctions as expert


SLpath = b2.Path()

fileList = ['../SLUntagged.dst.root']

ma.inputMdstList('default', fileList, path=SLpath)

stdPi('loose', path=SLpath)
stdK('loose', path=SLpath)
stdPi('all', path=SLpath)
stdE('all', path=SLpath)
stdMu('all', path=SLpath)

stdPi0s('loose', path=SLpath)  # for skim.standardlists.charm
stdPhotons('loose', path=SLpath)
stdKshorts(path=SLpath)

loadStdD0(path=SLpath)
loadStdDplus(path=SLpath)
loadStdDstar0(path=SLpath)
loadStdDstarPlus(path=SLpath)

# SL Skim
from skim.semileptonic import SemileptonicList
SLList = SemileptonicList(SLpath)
expert.skimOutputUdst('../SLUntagged', SLList, path=SLpath)

ma.summaryOfLists(SLList, path=SLpath)


expert.setSkimLogging(path=SLpath)
b2.process(SLpath)

# print out the summary
print(b2.statistics)
