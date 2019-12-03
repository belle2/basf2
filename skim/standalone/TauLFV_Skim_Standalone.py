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
from skim.standardlists.lightmesons import loadStdLightMesons
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('TauLFV')

taulfvskim = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
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

# Tau Skim
from skim.taupair import TauLFVList
tauList = TauLFVList(1, path=taulfvskim)

expert.skimOutputUdst(skimCode, tauList, path=taulfvskim)
ma.summaryOfLists(tauList, path=taulfvskim)

expert.setSkimLogging(path=taulfvskim)
b2.process(taulfvskim)

# print out the summary
print(b2.statistics)
