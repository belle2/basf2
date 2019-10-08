#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../feiSLBplus.dst.root</input>
    <output>../feiSLBplus.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from skimExpertFunctions import *


path = Path()

fileList = ['../feiSLBplus.dst.root']

inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforBplusSL(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B+ tag cuts
BplusSLList = BplusSL(path)
skimOutputUdst('../feiSLBplus', BplusSLList, path=path)
summaryOfLists(BplusSLList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
