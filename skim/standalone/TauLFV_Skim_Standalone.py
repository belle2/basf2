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
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'

skimCode = encodeSkimName('TauLFV')

fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList)

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

# Tau Skim
from skim.taupair import *
tauList = TauLFVList(1)

skimOutputUdst(skimCode, tauList)
summaryOfLists(tauList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
