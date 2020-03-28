#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau skims
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('TauLFV')

taulfvskim = b2.Path()

fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=taulfvskim)

stdE('all', path=taulfvskim)
stdMu('all', path=taulfvskim)
stdPi('all', path=taulfvskim)
stdK('all', path=taulfvskim)
stdPr('all', path=taulfvskim)
stdPhotons('all', path=taulfvskim)

# Tau Skim
from skim.taupair import TauLFVList
tauList = TauLFVList(1, path=taulfvskim)

expert.skimOutputUdst(skimCode, tauList, path=taulfvskim)
ma.summaryOfLists(tauList, path=taulfvskim)

expert.setSkimLogging(path=taulfvskim)
b2.process(taulfvskim)

# print out the summary
print(b2.statistics)
