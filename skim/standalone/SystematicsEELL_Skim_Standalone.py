#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Systematics Skims for four-lepton events
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdMu
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('SystematicsEELL')

skimpath = b2.Path()


fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=skimpath)

stdE('all', path=skimpath)
stdMu('all', path=skimpath)

from skim.systematics import EELLList
SysList = EELLList(skimpath)
expert.skimOutputUdst(skimCode, SysList, path=skimpath)
ma.summaryOfLists(SysList, path=skimpath)

expert.setSkimLogging(path=skimpath)
b2.process(skimpath)

print(b2.statistics)
