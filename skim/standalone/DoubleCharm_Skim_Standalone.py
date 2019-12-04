#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Double charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from skim.standardlists.charm import loadStdD0, loadStdDstar0, loadStdDplus, loadStdDstarPlus
import skimExpertFunctions as expert
b2.set_log_level(b2.LogLevel.INFO)


gb2_setuprel = 'release-04-00-00'
skimCode = expert.encodeSkimName('DoubleCharm')
fileList = expert.get_test_file("MC12_mixedBGx1")

path = b2.Path()
ma.inputMdstList('default', fileList, path=path)
stdPi('all', path=path)
stdPi('loose', path=path)
stdK('loose', path=path)
stdKshorts(path=path)
loadStdSkimPi0(path=path)
loadStdSkimPhoton(path=path)
stdPi0s('loose', path=path)
stdPhotons('loose', path=path)
loadStdD0(path=path)
loadStdDplus(path=path)
loadStdDstar0(path=path)
loadStdDstarPlus(path=path)

# Double Charm Skim
from skim.btocharm import DoubleCharmList
DCList = DoubleCharmList(path=path)
expert.skimOutputUdst(skimCode, DCList, path=path)
ma.summaryOfLists(DCList, path=path)
expert.setSkimLogging(path=path)
b2.process(path=path)

# print out the summary
print(b2.statistics)
