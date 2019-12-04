#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
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
gb2_setuprel = 'release-04-00-00'
b2.set_log_level(b2.LogLevel.INFO)
fileList = expert.get_test_file("MC12_mixedBGx1")
SLskimpath = b2.Path()

ma.inputMdstList('default', fileList, path=SLskimpath)
stdPi0s('loose', path=SLskimpath)
stdPhotons('loose', path=SLskimpath)
stdPi('loose', path=SLskimpath)
stdK('loose', path=SLskimpath)
stdPi('all', path=SLskimpath)
stdE('all', path=SLskimpath)
stdMu('all', path=SLskimpath)
loadStdSkimPi0(path=SLskimpath)
loadStdSkimPhoton(path=SLskimpath)
stdKshorts(path=SLskimpath)

loadStdD0(path=SLskimpath)
loadStdDplus(path=SLskimpath)
loadStdDstar0(path=SLskimpath)
loadStdDstarPlus(path=SLskimpath)

# SL Skim
from skim.semileptonic import SemileptonicList
SLList = SemileptonicList(SLskimpath)
skimCode1 = expert.encodeSkimName('SLUntagged')
print(skimCode1)
expert.skimOutputUdst(skimCode1, SLList, path=SLskimpath)
ma.summaryOfLists(SLList, path=SLskimpath)


from skim.leptonic import LeptonicList
lepList = LeptonicList(SLskimpath)
skimCode2 = expert.encodeSkimName('LeptonicUntagged')
print(skimCode2)
expert.skimOutputUdst(skimCode2, lepList, path=SLskimpath)
ma.summaryOfLists(lepList, path=SLskimpath)


from skim.semileptonic import PRList
PRList = PRList(path=SLskimpath)
skimCode3 = expert.encodeSkimName('PRsemileptonicUntagged')
expert.skimOutputUdst(skimCode3, PRList, path=SLskimpath)


ma.summaryOfLists(PRList, path=SLskimpath)
expert.setSkimLogging(path=SLskimpath)
b2.process(SLskimpath)

# print out the summary
print(b2.statistics)
