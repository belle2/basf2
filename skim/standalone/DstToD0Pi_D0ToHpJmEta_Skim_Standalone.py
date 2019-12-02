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
from stdV0s import mergedKshorts, stdKshorts
from stdPi0s import stdPi0s
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
b2.set_log_level(LogLevel.INFO)
import sys
import os
import glob
skimCode = expert.encodeSkimName('DstToD0Pi_D0ToHpJmEta')

c3bh3path = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=c3bh3path)


loadStdSkimPhoton(path=c3bh3path)
loadStdSkimPi0(path=c3bh3path)
stdPi('loose', path=c3bh3path)
stdK('loose', path=c3bh3path)
stdE('loose', path=c3bh3path)
stdMu('loose', path=c3bh3path)
stdPi('all', path=c3bh3path)
stdK('all', path=c3bh3path)
stdE('all', path=c3bh3path)
stdMu('all', path=c3bh3path)
stdKshorts(path=c3bh3path)

from skim.charm import DstToD0PiD0ToHpJmEta
DstToD0PiD0ToHpJmEtaList = DstToD0PiD0ToHpJmEta(c3bh3path)
expert.skimOutputUdst(skimCode, DstToD0PiD0ToHpJmEtaList, path=c3bh3path)

ma.summaryOfLists(DstToD0PiD0ToHpJmEtaList, path=c3bh3path)


expert.setSkimLogging(path=c3bh3path)
b2.process(c3bh3path)

print(statistics)
