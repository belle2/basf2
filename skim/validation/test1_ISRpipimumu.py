#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../ISRpipimumu.dst.root</input>
    <output>../ISRpipimumu.udst.root</output>
    <contact>jiasen@buaa.edu.cn</contact>
    <interval>nightly</interval>
</header>
"""

__author__ = "S. Jia"


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from stdCharged import stdE, stdK, stdMu, stdPi

# create a new path
ISRskimpath = b2.Path()

fileList = ['../ISRpipimumu.dst.root']

ma.inputMdstList('default', fileList, path=ISRskimpath)

# use standard final state particle lists
stdPi('loose', path=ISRskimpath)
stdK('loose', path=ISRskimpath)
stdE('loose', path=ISRskimpath)
stdMu('loose', path=ISRskimpath)
stdPi('all', path=ISRskimpath)
stdK('all', path=ISRskimpath)
stdE('all', path=ISRskimpath)
stdMu('all', path=ISRskimpath)

# importing the reconstructed events from the ISRpipicc_List file
from skim.quarkonium import ISRpipiccList
ISRpipicc = ISRpipiccList(path=ISRskimpath)

# output to Udst file
expert.skimOutputUdst('../ISRpipimumu.udst.root', ISRpipicc, path=ISRskimpath)

# print out Particle List statistics
ma.summaryOfLists(ISRpipicc, path=ISRskimpath)

# output skim log information
expert.setSkimLogging(path=ISRskimpath)

# process the path
b2.process(ISRskimpath)

# print out the summary
print(b2.statistics)
