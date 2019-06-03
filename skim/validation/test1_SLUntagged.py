#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../SLUntagged.dst.root</input>
    <output>../SLUntagged.udst.root</output>
    <contact>hannah.wakeling@mail.mcgill.ca, philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skimExpertFunctions import *


SLpath = Path()

fileList = ['../SLUntagged.dst.root']

inputMdstList('default', fileList, path=SLpath)

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
skimOutputUdst('../SLUntagged', SLList, path=SLpath)

summaryOfLists(SLList, path=SLpath)


setSkimLogging(path=SLpath)
process(SLpath)

# print out the summary
print(statistics)
