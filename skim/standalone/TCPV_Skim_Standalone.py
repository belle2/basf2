#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 29/Sep/2016
#
#####################################################

__author__ = " Reem Rasheed"


from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import loadStdDiLeptons
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'
set_log_level(LogLevel.INFO)


import sys
import os
import glob
skimCode = encodeSkimName('TCPV')

# create a path
path = Path()

fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=path)

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
cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=path)

# TCPV Skim
from skim.tcpv import TCPVList
tcpvList = TCPVList(path=path)
skimOutputUdst(skimCode, tcpvList, path=path)
summaryOfLists(tcpvList, path=path)

setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
