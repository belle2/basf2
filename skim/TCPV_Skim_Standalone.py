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
gb2_setuprel = 'release-01-00-00'
set_log_level(LogLevel.INFO)


import sys
import os
import glob
fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]


inputMdstList('default', fileList)

loadStdSkimPi0()
loadStdSkimPhoton()
stdPi0s('loose')
loadStdCharged()
stdPhotons('loose')
loadStdKS()
loadStdDiLeptons(True)
loadStdLightMesons()

# TCPV Skim
from TCPV_List import *
tcpvList = TCPVList()
skimOutputUdst('TCPV', tcpvList)
summaryOfLists(tcpvList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
