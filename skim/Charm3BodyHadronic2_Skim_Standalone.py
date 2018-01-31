#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdV0s import *

from skimExpertFunctions import *
gb2_setuprel = 'release-01-00-00'
set_log_level(LogLevel.INFO)
import sys
import os
import glob

ccbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_00051*_prod00000198_task0000051*.root']

inputMdstList('default', fileList)

stdKshorts()
loadStdCharged()
from Charm3BodyHadronic2_List import *


DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs()
skimOutputUdst('Charm3BodyHadronic2', DstToD0PiD0ToHpHmKsList)

summaryOfLists(DstToD0PiD0ToHpHmKsList)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

print(statistics)
