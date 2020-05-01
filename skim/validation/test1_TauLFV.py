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
from skim.standardlists.lightmesons import loadStdAllRho0, loadStdAllKstar0, loadStdAllPhi, loadStdAllF_0

taulfvskim = b2.Path()

fileList = ['../TauLFV.dst.root']
ma.inputMdstList('default', fileList, path=taulfvskim)

stdE('all', path=taulfvskim)
stdMu('all', path=taulfvskim)
stdPi('all', path=taulfvskim)
stdK('all', path=taulfvskim)
stdPr('all', path=taulfvskim)
stdPhotons('all', path=taulfvskim)
loadStdAllRho0(path=taulfvskim)
loadStdAllKstar0(path=taulfvskim)
loadStdAllPhi(path=taulfvskim)
loadStdAllF_0(path=taulfvskim)

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
