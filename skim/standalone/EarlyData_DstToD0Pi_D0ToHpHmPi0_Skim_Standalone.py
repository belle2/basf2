#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
import skimExpertFunctions as expert

gb2_setuprel = 'release-04-00-00'
b2.set_log_level(b2.LogLevel.INFO)
skimCode = expert.encodeSkimName('DstToD0Pi_D0ToHpHmPi0')

c3bh1path = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=c3bh1path)

loadStdSkimPhoton(path=c3bh1path)
loadStdSkimPi0(path=c3bh1path)
stdPi('loose', path=c3bh1path)
stdK('loose', path=c3bh1path)
stdE('loose', path=c3bh1path)
stdMu('loose', path=c3bh1path)
stdPi('all', path=c3bh1path)
stdK('all', path=c3bh1path)
stdE('all', path=c3bh1path)
stdMu('all', path=c3bh1path)

from skim.charm import EarlyData_DstToD0PiD0ToHpHmPi0
DstToD0PiD0ToHpHmPi0List = EarlyData_DstToD0PiD0ToHpHmPi0(path=c3bh1path)
expert.skimOutputUdst(skimCode, DstToD0PiD0ToHpHmPi0List, path=c3bh1path)

ma.summaryOfLists(DstToD0PiD0ToHpHmPi0List, path=c3bh1path)


expert.setSkimLogging(path=c3bh1path)
b2.process(c3bh1path)

print(statistics)
