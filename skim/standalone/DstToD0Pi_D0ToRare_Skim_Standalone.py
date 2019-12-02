#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# Charm skims
# P. Urquijo, 6/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdMu, stdPi
from stdPhotons import loadStdSkimPhoton
import skimExpertFunctions as expert


gb2_setuprel = 'release-04-00-00'
b2.set_log_level(LogLevel.INFO)

# skimCode = expert.encodeSkimName('DstToD0Pi_D0ToRare')
skimCode = "DstToD0Pi_D0ToRare"

crpath = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=crpath)


loadStdSkimPi0(path=crpath)
loadStdSkimPhoton(path=crpath)
stdMu('loose', path=crpath)
stdE('loose', path=crpath)
stdPi('loose', path=crpath)

from skim.charm import CharmRare
DstToD0Pi_D0ToRareList = CharmRare(crpath)
expert.skimOutputUdst(skimCode, DstToD0Pi_D0ToRareList, path=crpath)
ma.summaryOfLists(DstToD0Pi_D0ToRareList, path=crpath)

expert.setSkimLogging(path=crpath)
b2.process(crpath)

print(statistics)
