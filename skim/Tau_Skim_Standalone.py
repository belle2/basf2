
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

gb2_setuprel = 'release-01-00-00'
import sys
import os
import glob

fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]


inputMdstList('default', fileList)

stdPi0s('loose')
loadStdCharged()
loadStdSkimPhoton()
loadStdSkimPi0()
loadStdKS()
stdPhotons('loose')
loadStdLightMesons()

# Tau Skim
from Tau_List import *
tauList = TauLFVList()

skimOutputUdst('Tau', tauList)
summaryOfLists(tauList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
