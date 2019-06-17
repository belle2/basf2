#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau skims
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu, stdPr
from stdPhotons import *
from skim.standardlists.lightmesons import *
from stdPi0s import *
from stdV0s import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

skimCode = encodeSkimName('TauLFV')

taulfvskim = Path()

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=taulfvskim)

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
from skim.taupair import *
tauList = TauLFVList(1, path=taulfvskim)

skimOutputUdst(skimCode, tauList, path=taulfvskim)
summaryOfLists(tauList, path=taulfvskim)

setSkimLogging(path=taulfvskim)
process(taulfvskim)

# print out the summary
print(statistics)
