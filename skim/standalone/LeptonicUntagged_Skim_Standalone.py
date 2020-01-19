#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
# P. Urquijo, 6/Jan/2015
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdMu
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'
skimCode = expert.encodeSkimName('LeptonicUntagged')

fileList = expert.get_test_file("MC12_mixedBGx1")

leppath = b2.Path()

ma.inputMdstList('default', fileList, path=leppath)

stdE('all', path=leppath)
stdMu('all', path=leppath)

# SL Skim
from skim.leptonic import LeptonicList

lepList = LeptonicList(path=leppath)
expert.skimOutputUdst(skimCode, lepList, path=leppath)

ma.summaryOfLists(lepList, path=leppath)


expert.setSkimLogging(path=leppath)
b2.process(leppath)

# print out the summary
print(b2.statistics)
