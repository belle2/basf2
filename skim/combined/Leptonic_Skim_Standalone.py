#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdMu
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)
fileList = expert.get_test_file("MC12_mixedBGx1")
SLskimpath = b2.Path()

ma.inputMdstList('default', fileList, path=SLskimpath)

stdE('all', path=SLskimpath)
stdMu('all', path=SLskimpath)

# LeptonicUntagged skim
from skim.leptonic import LeptonicList
leptonicList = LeptonicList(SLskimpath)
skimCode = expert.encodeSkimName('LeptonicUntagged')
expert.skimOutputUdst(skimCode, leptonicList, path=SLskimpath)
ma.summaryOfLists(leptonicList, path=SLskimpath)


expert.setSkimLogging(path=SLskimpath)
b2.process(SLskimpath)

# print out the summary
print(b2.statistics)
