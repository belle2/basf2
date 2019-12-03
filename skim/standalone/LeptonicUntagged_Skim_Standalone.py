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
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from skim.standardlists.charm import loadStdD0, loadStdDstar0, loadStdDplus, loadStdDstarPlus
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'
skimCode = expert.encodeSkimName('LeptonicUntagged')

fileList = expert.get_test_file("mixedBGx1", "MC12")

leppath = b2.Path()

ma.inputMdstList('default', fileList, path=leppath)

loadStdSkimPi0(path=leppath)
loadStdSkimPhoton(path=leppath)
stdPi('loose', path=leppath)
stdK('loose', path=leppath)
stdPi('all', path=leppath)
stdE('all', path=leppath)
stdMu('all', path=leppath)
stdPi0s('loose', path=leppath)  # for stdCharm.py
stdPhotons('loose', path=leppath)
stdKshorts(path=leppath)

loadStdD0(path=leppath)
loadStdDplus(path=leppath)
loadStdDstar0(path=leppath)
loadStdDstarPlus(path=leppath)

# SL Skim
from skim.leptonic import LeptonicList

lepList = LeptonicList(path=leppath)
expert.skimOutputUdst(skimCode, lepList, path=leppath)

ma.summaryOfLists(lepList, path=leppath)


expert.setSkimLogging(path=leppath)
b2.process(leppath)

# print out the summary
print(b2.statistics)
