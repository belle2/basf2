#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 29/Sep/2016
#
#####################################################

__author__ = " Reem Rasheed"


import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdLightMesons
from skim.standardlists.dileptons import loadStdDiLeptons
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
b2.set_log_level(b2.LogLevel.INFO)


skimCode = expert.encodeSkimName('TCPV')

# create a path
path = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")

ma.inputMdstList('default', fileList, path=path)

loadStdSkimPi0(path=path)
loadStdSkimPhoton(path=path)
stdPi0s('loose', path=path)
stdPi('loose', path=path)
stdK('loose', path=path)
stdE('loose', path=path)
stdMu('loose', path=path)
stdPi('all', path=path)
stdPhotons('loose', path=path)
stdKshorts(path=path)
loadStdDiLeptons(True, path=path)
loadStdLightMesons(path=path)
ma.cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=path)

# TCPV Skim
from skim.tcpv import TCPVList
tcpvList = TCPVList(path=path)
expert.skimOutputUdst(skimCode, tcpvList, path=path)
ma.summaryOfLists(tcpvList, path=path)

expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
