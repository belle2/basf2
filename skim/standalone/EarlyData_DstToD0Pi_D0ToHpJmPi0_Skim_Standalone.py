#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPi0s import loadStdSkimPi0
from stdPhotons import loadStdSkimPhoton
import skimExpertFunctions as expert


gb2_setuprel = 'release-04-00-00'
b2.set_log_level(b2.LogLevel.INFO)
skimCode = expert.encodeSkimName('DstToD0Pi_D0ToHpJmPi0')

c3bhpath = b2.Path()
fileList = expert.get_test_file("ddbarBGx0", "MC12")
ma.inputMdstList('default', fileList, path=c3bhpath)


loadStdSkimPhoton(path=c3bhpath)
loadStdSkimPi0(path=c3bhpath)
stdPi('loose', path=c3bhpath)
stdK('loose', path=c3bhpath)
stdE('loose', path=c3bhpath)
stdMu('loose', path=c3bhpath)
stdPi('all', path=c3bhpath)
stdK('all', path=c3bhpath)
stdE('all', path=c3bhpath)
stdMu('all', path=c3bhpath)

from skim.charm import EarlyData_DstToD0PiD0ToHpJmPi0
DstToD0PiD0ToHpJmPi0List = EarlyData_DstToD0PiD0ToHpJmPi0(c3bhpath)
expert.skimOutputUdst(skimCode, DstToD0PiD0ToHpJmPi0List, path=c3bhpath)

ma.summaryOfLists(DstToD0PiD0ToHpJmPi0List, path=c3bhpath)


expert.setSkimLogging(path=c3bhpath)
b2.process(c3bhpath)

print(b2.statistics)
