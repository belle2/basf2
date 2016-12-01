#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau skims
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdLightMesons import *
from stdPi0s import *
from stdV0s import *

set_log_level(LogLevel.INFO)

filelist = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'
     ]
inputMdstList('default', filelist)

loadStdCharged()
loadStdPhoton()
loadStdPi0()
loadStdKS()
loadStdLightMesons()


# Tau Skim
from Tau_List import *
tauList = TauLFVList()

skimOutputUdst('TauLFV_Standalone', tauList)
summaryOfLists(tauList)

process(analysis_main)

# print out the summary
print(statistics)
