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
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import *

fileList = ['../TauLFV.dst.root']

inputMdstList('MC9', fileList)

stdPi('loose')
stdK('loose')
stdPr('loose')
stdE('loose')
stdMu('loose')
stdPhotons('loose')
stdPi0s('loose')
loadStdSkimPi0()
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
