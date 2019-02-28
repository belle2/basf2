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

import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from skimExpertFunctions import *
from stdCharged import *
from stdPhotons import *

taugenericskim = Path()

fileList = ['../TauGeneric.dst.root']
inputMdstList('MC9', fileList, path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# TauGeneric skim
from skim.taupair import *
tauList = TauList(path=taugenericskim)
skimOutputUdst('../TauGeneric.udst.root', tauList, path=taugenericskim)
summaryOfLists(tauList, path=taugenericskim)

# Suppress noisy modules, and then process
setSkimLogging(path=taugenericskim)
process(taugenericskim)

# print out the summary
print(statistics)
