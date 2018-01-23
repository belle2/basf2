#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 29/Sep/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from stdLightMesons import *
from stdDiLeptons import *
gb2_setuprel = 'build-2017-10-16'
set_log_level(LogLevel.INFO)


import sys
import os
import glob
fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]


inputMdstList('default', fileList)
stdPhotons('loose')
stdPi0s('loose')
loadStdSkimPi0()
loadStdSkimPhoton()
loadStdCharged()
loadStdKS()
loadStdDiLeptons(True)
loadStdLightMesons()

# TCPV Skim
from TCPV_List import *
tcpvList = TCPVList()
skimOutputUdst('TCPV', tcpvList)
summaryOfLists(tcpvList)

process(analysis_main)

# print out the summary
print(statistics)
