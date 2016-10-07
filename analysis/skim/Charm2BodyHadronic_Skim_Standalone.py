#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdFSParticles import *

set_log_level(LogLevel.INFO)

filelist = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_0005*_prod00000198_task000005*.root'
     ]

inputMdstList('default', filelist)

stdFSParticles()
loadStdCharged()

from Charm2BodyHadronic_List import *
Charm2BodyChargedList = Charm2BodyCharged()
skimOutputUdst('Charm2BodyCharged_Standalone', Charm2BodyChargedList)
summaryOfLists(Charm2BodyChargedList)

process(analysis_main)

print(statistics)
