#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TauThrust.dst.root</input>
  <output>../TauThrust.udst.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

__author__ = "Kenji Inami"


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from stdCharged import *
from stdPhotons import *

tauthrustskim = b2.Path()

fileList = ['../TauThrust.dst.root']
ma.inputMdstList('default', fileList, path=tauthrustskim)

stdPi('all', path=tauthrustskim)
stdPhotons('all', path=tauthrustskim)

# TauThrust skim
from skim.taupair import *
tauList = TauThrustList(path=tauthrustskim)
expert.skimOutputUdst('../TauThrust.udst.root', tauList, path=tauthrustskim)
ma.summaryOfLists(tauList, path=tauthrustskim)

# Suppress noisy modules, and then process
expert.setSkimLogging(path=tauthrustskim)
b2.process(tauthrustskim)

# print out the summary
print(b2.statistics)
