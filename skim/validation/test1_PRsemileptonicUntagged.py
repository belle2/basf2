#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../PRsemileptonicUntagged.dst.root</input>
    <output>../PRsemileptonic.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skimExpertFunctions import *
from skim.standardlists.charm import *


PRSLpath = Path()

fileList = ['../PRsemileptonicUntagged.dst.root']

inputMdstList('default', fileList, path=PRSLpath)

# Load particle lists
stdPi('all', path=PRSLpath)
stdE('all', path=PRSLpath)
stdMu('all', path=PRSLpath)

# PR Skim
from skim.semileptonic import PRList
PRList = PRList(path=PRSLpath)
skimOutputUdst('../PRsemileptonicUntagged', PRList, path=PRSLpath)

summaryOfLists(PRList, path=PRSLpath)


setSkimLogging(path=PRSLpath)
process(path=PRSLpath)
# print out the summary
print(statistics)
