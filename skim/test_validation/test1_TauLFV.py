#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TauLFV.dst.root</input>
  <output>../TauLFV.udst.root</output>
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
from stdLightMesons import *
from stdPi0s import *
from stdV0s import *

fileList = ['../TauLFV.dst.root']

inputMdstList('MC9', fileList)

loadStdCharged()
stdPhotons('loose')
loadStdSkimPi0()
stdPi0s('loose')
stdKshorts()
loadStdLightMesons()

# TauLFV skim
from skim.taupair import *
tauList = TauLFVList(1)
skimOutputUdst('../TauLFV.udst.root', tauList)
summaryOfLists(tauList)

# Suppress noisy modules, and then process
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
