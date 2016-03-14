#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from stdLightMesons import *

set_log_level(LogLevel.INFO)

filelist = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'
     ]
inputMdstList(filelist)

loadStdCharged()
loadStdPhoton()
loadStdPi0()
loadStdLightMesons(True)
loadStdKS()

# TCPV Skim
from TCPV_List import *
tcpvList = TCPVList()
removeParticlesNotInLists(tcpvList)
skimOutputUdst('TCPV_Standalone', tcpvList)
summaryOfLists(tcpvList)

process(analysis_main)

# print out the summary
print(statistics)
