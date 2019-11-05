#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# Charm skims
# P. Urquijo, 6/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu, stdPi
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file


gb2_setuprel = 'release-04-00-00'
set_log_level(LogLevel.INFO)
import os
import sys
import glob

# skimCode = encodeSkimName('DstToD0Pi_D0ToRare')
skimCode = "DstToD0Pi_D0ToRare"

crpath = Path()

fileList = get_test_file("mixedBGx1_MC12")
inputMdstList('default', fileList, path=crpath)


loadStdSkimPi0(path=crpath)
loadStdSkimPhoton(path=crpath)
stdMu('loose', path=crpath)
stdE('loose', path=crpath)
stdPi('loose', path=crpath)

from skim.charm import CharmRare
DstToD0Pi_D0ToRareList = CharmRare(crpath)
skimOutputUdst(skimCode, DstToD0Pi_D0ToRareList, path=crpath)
summaryOfLists(DstToD0Pi_D0ToRareList, path=crpath)

setSkimLogging(path=crpath)
process(crpath)

print(statistics)
