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
import skimExpertFunctions as expert
import stdV0s

gb2_setuprel = 'release-04-01-00'
b2.set_log_level(b2.LogLevel.INFO)

skimCode = expert.encodeSkimName('DstToD0Pi_D0ToHpHmKs')

c3bh2path = b2.Path()

fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=c3bh2path)

stdPi('all', path=c3bh2path)
stdK('all', path=c3bh2path)
stdV0s.stdKshorts(path=c3bh2path)

from skim.charm import DstToD0PiD0ToHpHmKs
DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs(c3bh2path)
expert.skimOutputUdst(skimCode, DstToD0PiD0ToHpHmKsList, path=c3bh2path)

ma.summaryOfLists(DstToD0PiD0ToHpHmKsList, path=c3bh2path)


expert.setSkimLogging(path=c3bh2path)
b2.process(c3bh2path)

print(b2.statistics)
