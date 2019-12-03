#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../PRsemileptonicUntagged.dst.root</input>
    <output>../PRsemileptonicUntagged.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdE, stdMu
import skimExpertFunctions as expert


PRSLpath = b2.Path()

fileList = ['../PRsemileptonicUntagged.dst.root']

ma.inputMdstList('default', fileList, path=PRSLpath)

# Load particle lists
stdPi('all', path=PRSLpath)
stdE('all', path=PRSLpath)
stdMu('all', path=PRSLpath)

# PR Skim
from skim.semileptonic import PRList
PRList = PRList(path=PRSLpath)
expert.skimOutputUdst('../PRsemileptonicUntagged', PRList, path=PRSLpath)

ma.summaryOfLists(PRList, path=PRSLpath)


expert.setSkimLogging(path=PRSLpath)
b2.process(path=PRSLpath)
# print out the summary
print(b2.statistics)
