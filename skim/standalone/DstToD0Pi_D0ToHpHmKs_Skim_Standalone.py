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
import skimExpertFunctions as expert

gb2_setuprel = 'release-04-00-00'
b2.set_log_level(LogLevel.INFO)

skimCode = expert.encodeSkimName('DstToD0Pi_D0ToHpHmKs')

c3bh2path = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=c3bh2path)


stdKshorts(path=c3bh2path)
mergedKshorts(path=c3bh2path)
stdPi('loose', path=c3bh2path)
stdK('loose', path=c3bh2path)
stdE('loose', path=c3bh2path)
stdMu('loose', path=c3bh2path)
stdPi('all', path=c3bh2path)
stdK('all', path=c3bh2path)
stdE('all', path=c3bh2path)
stdMu('all', path=c3bh2path)

from skim.charm import DstToD0PiD0ToHpHmKs
DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs(c3bh2path)
expert.skimOutputUdst(skimCode, DstToD0PiD0ToHpHmKsList, path=c3bh2path)

ma.summaryOfLists(DstToD0PiD0ToHpHmKsList, path=c3bh2path)


expert.setSkimLogging(path=c3bh2path)
b2.process(c3bh2path)

print(statistics)
