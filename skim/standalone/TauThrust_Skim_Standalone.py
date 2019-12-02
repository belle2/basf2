#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau skim using thrust
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi
from stdPhotons import stdPhotons
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('TauThrust')

tauthrustskim = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")

ma.inputMdstList('default', fileList, path=tauthrustskim)

stdPi('all', path=tauthrustskim)
stdPhotons('all', path=tauthrustskim)

# Tau Skim
from skim.taupair import *
tauList = TauThrustList(path=tauthrustskim)

expert.skimOutputUdst(skimCode, tauList, path=tauthrustskim)
ma.summaryOfLists(tauList, path=tauthrustskim)

expert.setSkimLogging(path=tauthrustskim)
b2.process(tauthrustskim)

# print out the summary
print(statistics)
