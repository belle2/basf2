#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../feiHadronicBplus.dst.root</input>
    <output>../feiHadronicBplus.udst.root</output>
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

fileList = ['../feiHadronicBplus.dst.root']

inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforBplusHadronic(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)

# Apply final B+ tag cuts
BplushadronicList = BplusHadronic(path)
skimOutputUdst('../feiHadronicBplus', BplushadronicList, path=path)
summaryOfLists(BplushadronicList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
