#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau generic skims
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import *
from stdPhotons import *
import skimExpertFunctions as expert
b2.set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('TauGeneric')

taugenericskim = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")

ma.inputMdstList('default', fileList, path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# Tau Skim
from skim.taupair import *
tauList = TauList(path=taugenericskim)

expert.skimOutputUdst(skimCode, tauList, path=taugenericskim)
ma.summaryOfLists(tauList, path=taugenericskim)

expert.setSkimLogging(path=taugenericskim)
b2.process(taugenericskim)

# print out the summary
print(statistics)
