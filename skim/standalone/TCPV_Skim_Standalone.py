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
from skimExpertFunctions import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import loadStdDiLeptons

gb2_setuprel = 'release-03-00-03'
set_log_level(LogLevel.INFO)


import sys
import os
import glob
skimCode = encodeSkimName('TCPV')

# create a path
tcpvskimpath = Path()

fileList = get_test_file("mixedBGx1", "MC11")

inputMdstList('default', fileList, path=tcpvskimpath)

loadStdSkimPi0(path=tcpvskimpath)
loadStdSkimPhoton(path=tcpvskimpath)
stdPi0s('loose', path=tcpvskimpath)
stdPi('loose', path=tcpvskimpath)
stdK('loose', path=tcpvskimpath)
stdE('loose', path=tcpvskimpath)
stdMu('loose', path=tcpvskimpath)
stdPi('all', path=tcpvskimpath)
stdPhotons('loose', path=tcpvskimpath)
stdKshorts(path=tcpvskimpath)
loadStdDiLeptons(True, path=tcpvskimpath)
loadStdLightMesons(path=tcpvskimpath)
cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=tcpvskimpath)

# TCPV Skim
from skim.tcpv import TCPVList
tcpvList = TCPVList(path=tcpvskimpath)
skimOutputUdst(skimCode, tcpvList, path=tcpvskimpath)
summaryOfLists(tcpvList, path=tcpvskimpath)

setSkimLogging()
process(tcpvskimpath)

# print out the summary
print(statistics)
