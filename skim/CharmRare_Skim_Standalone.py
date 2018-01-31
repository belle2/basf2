#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdPhotons import *
gb2_setuprel = 'release-01-00-00'
set_log_level(LogLevel.INFO)
import sys
import os
import glob

fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_0005*_prod00000198_task000005*.root'
     ]
inputMdstList('default', fileList)


loadStdSkimPi0()
loadStdSkimPhoton()
stdLooseMu()
stdLooseE()
from CharmRare_List import *
CharmRareList = CharmRareList()
skimOutputUdst('CharmRare', CharmRareList)
summaryOfLists(CharmRareList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

print(statistics)
