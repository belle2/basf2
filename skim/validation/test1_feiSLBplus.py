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

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert


path = b2.Path()

fileList = ['../feiSLBplus.dst.root']

ma.inputMdstList('default', fileList, path=path)

from skim.fei import BplusSL, runFEIforBplusSL
# run pre-selection cuts and FEI
runFEIforBplusSL(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B+ tag cuts
BplusSLList = BplusSL(path)
expert.skimOutputUdst('../feiSLBplus', BplusSLList, path=path)
ma.summaryOfLists(BplusSLList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
