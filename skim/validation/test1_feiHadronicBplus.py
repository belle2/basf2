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

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert


path = b2.Path()

fileList = ['../feiHadronicBplus.dst.root']

ma.inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforBplusHadronic(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)

# Apply final B+ tag cuts
BplusHadronicList = BplusHadronic(path)
expert.skimOutputUdst('../feiHadronicBplus', BplusHadronicList, path=path)
ma.summaryOfLists(BplusHadronicList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
