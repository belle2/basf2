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
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons
from skim.standardlists.charm import loadStdD0, loadStdDstar0, loadStdDplus, loadStdDstarPlus
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
b2.set_log_level(b2.LogLevel.INFO)

skimCode = expert.encodeSkimName('SLUntagged')

SLpath = b2.Path()
fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=SLpath)

stdPi('loose', path=SLpath)
stdK('all', path=SLpath)
stdPi('all', path=SLpath)
stdE('all', path=SLpath)
stdMu('all', path=SLpath)

loadStdPi0ForBToHadrons(path=SLpath)  # For skim.standardlists.charm lists
stdPi0s('loose', path=SLpath)
stdPhotons('loose', path=SLpath)
stdKshorts(path=SLpath)

loadStdD0(path=SLpath)
loadStdDplus(path=SLpath)
loadStdDstar0(path=SLpath)
loadStdDstarPlus(path=SLpath)

# SL Skim
from skim.semileptonic import SemileptonicList
SLList = SemileptonicList(SLpath)
expert.skimOutputUdst(skimCode, SLList, path=SLpath)

ma.summaryOfLists(SLList, path=SLpath)


expert.setSkimLogging(path=SLpath)
b2.process(SLpath)

# print out the summary
print(b2.statistics)
