#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TauGeneric.dst.root</input>
  <output>../TauGeneric.udst.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

__author__ = "Kenji Inami"


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from stdCharged import *
from stdPhotons import *

taugenericskim = b2.Path()

fileList = ['../TauGeneric.dst.root']
ma.inputMdstList('default', fileList, path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# TauGeneric skim
from skim.taupair import *
tauList = TauList(path=taugenericskim)
expert.skimOutputUdst('../TauGeneric.udst.root', tauList, path=taugenericskim)
ma.summaryOfLists(tauList, path=taugenericskim)

# Suppress noisy modules, and then process
expert.setSkimLogging(path=taugenericskim)
b2.process(taugenericskim)

# print out the summary
print(b2.statistics)
