#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau generic skims
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'

skimCode = encodeSkimName('TauGeneric')

taugenericskim = Path()

fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# Tau Skim
from skim.taupair import *
tauList = TauList(path=taugenericskim)

skimOutputUdst(skimCode, tauList, path=taugenericskim)
summaryOfLists(tauList, path=taugenericskim)

setSkimLogging(path=taugenericskim)
process(taugenericskim)

# print out the summary
print(statistics)
