#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Prepare all skims at once
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0 import *
from stdKS import *
from stdCharm import *
from stdLightMesons import *

set_log_level(LogLevel.INFO)

filelist = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'
     ]
inputMdstList(filelist)

loadStdCharged()
loadStdPi0()
loadStdKS()
loadStdLightMesons()
loadStdPhoton()
loadStdPhotonE15()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# Double Charm Skim
from DoubleCharm_List import *
DCList = DoubleCharmList()
skimOutputUdst('DC', DCList)
summaryOfLists(DCList)

# Tau Skim
from Tau_List import *
tauList = TauLFVList()
skimOutputUdst('TauLFV', tauList)
summaryOfLists(tauList)

# EWP Skim
from EWP_List import *
XgammaList = B2XgammaList()
skimOutputUdst('BtoXgamma', XgammaList)
summaryOfLists(XgammaList)

XllList = B2XllList()
skimOutputUdst('BtoXll', XllList)
summaryOfLists(XllList)

# Had Skim
from CharmlessHad_List import *
HadList = CharmlessHadList()
skimOutputUdst('CharmlessHad', HadList)
summaryOfLists(HadList)

# TCPV Skim
from TCPV_List import *
tcpvList = TCPVList()
skimOutputUdst('TCPV', tcpvList)
summaryOfLists(tcpvList)

# SL Skim
from Semileptonic_List import *
SLList = SemileptonicList()
skimOutputUdst('SLUntagged', SLList)
summaryOfLists(SLList)

lepList = LeptonicList()
skimOutputUdst('LeptonicUntagged', lepList)
summaryOfLists(lepList)

# Charm Skim
from Charm_List import *
CharmList = CharmRareList()
skimOutputUdst('CharmRare', CharmList)
summaryOfLists(CharmList)

# FEI Skim

process(analysis_main)

# print out the summary
print(statistics)
