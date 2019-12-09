#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Skims for LFV Z'
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'
skimCode = expert.encodeSkimName('LFVZpVisible')

lfvzppath = b2.Path()
fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=lfvzppath)

stdPi('loose', path=lfvzppath)
stdK('loose', path=lfvzppath)
stdE('loose', path=lfvzppath)
stdMu('loose', path=lfvzppath)
stdPi('all', path=lfvzppath)
stdK('all', path=lfvzppath)
stdE('all', path=lfvzppath)
stdMu('all', path=lfvzppath)


from skim.dark import LFVZpVisibleList
SysList = LFVZpVisibleList(path=lfvzppath)
expert.skimOutputUdst(skimCode, SysList, path=lfvzppath)
ma.summaryOfLists(SysList, path=lfvzppath)

expert.setSkimLogging(path=lfvzppath)
b2.process(lfvzppath)

print(b2.statistics)
