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

import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from skimExpertFunctions import *
from stdCharged import *
from stdPhotons import *

tauthrustskim = Path()

fileList = ['../TauThrust.dst.root']
inputMdstList('MC9', fileList, path=tauthrustskim)

stdPi('all', path=tauthrustskim)
stdPhotons('all', path=tauthrustskim)

# TauThrust skim
from skim.taupair import *
tauList = TauThrustList(path=tauthrustskim)
skimOutputUdst('../TauThrust.udst.root', tauList, path=tauthrustskim)
summaryOfLists(tauList, path=tauthrustskim)

# Suppress noisy modules, and then process
setSkimLogging(path=tauthrustskim)
process(tauthrustskim)

# print out the summary
print(statistics)
