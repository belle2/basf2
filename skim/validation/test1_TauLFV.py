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


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdLightMesons

taulfvskim = b2.Path()

fileList = ['../TauLFV.dst.root']
ma.inputMdstList('default', fileList, path=taulfvskim)

stdPi('loose', path=taulfvskim)
stdK('loose', path=taulfvskim)
stdPr('loose', path=taulfvskim)
stdE('loose', path=taulfvskim)
stdMu('loose', path=taulfvskim)
stdPhotons('loose', path=taulfvskim)
stdPi0s('loose', path=taulfvskim)
loadStdSkimPi0(path=taulfvskim)
stdKshorts(path=taulfvskim)
loadStdLightMesons(path=taulfvskim)

# TauLFV skim
from skim.taupair import TauLFVList
tauList = TauLFVList(1, path=taulfvskim)
expert.skimOutputUdst('../TauLFV.udst.root', tauList, path=taulfvskim)
ma.summaryOfLists(tauList, path=taulfvskim)

# Suppress noisy modules, and then process
expert.setSkimLogging(path=taulfvskim)
b2.process(taulfvskim)

# print out the summary
print(b2.statistics)
