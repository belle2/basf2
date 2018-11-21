#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 29/Sep/2016
#
######################################################
"""
Time dependent CP violation skim standalone for the analysis of TDCPV analysis in WG3 .
Physics channels : bd → qqs and bd → ccs .
Skim code : 13160100
for analysis users we will reconstruct again in the analysis steering file
The skim uses standard particles from analysis package , except one list of gamma:E15,
of specific energy range, for 'B0 -> Kshort pi+ pi- gamma' channel.

Particle lists used :

phi:loose
k_S0:all
eta:loose
pi0:loose
pi0:skim
rho0:loose
pi+:all
gamma:E15
omega:loose
J/psi: eeLoose
J/psi: mumuLoose
psi(2S): eeLoose
psi(2S): mumuloose
K*0:loose
phi:loose
"""
__author__ = " Reem Rasheed"


from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
set_log_level(LogLevel.INFO)


import sys
import os
import glob
skimCode = encodeSkimName('TCPV')


# create a path
tcpvskimpath = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList, path=tcpvskimpath)

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
