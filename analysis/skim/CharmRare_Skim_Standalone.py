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
from stdFSParticles import *
from stdCharged import *
from stdPi0s import *

set_log_level(LogLevel.INFO)

filelist = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_0005*_prod00000198_task000005*.root'
     ]

inputMdstList('default', filelist)


stdFSParticles()
# cutAndCopyList('pi0:loose', 'pi0:all', '-0.6 < extraInfo(BDT) < 1.0', True)
loadStdLooseMu()
loadStdLooseE()
loadStdPi0()

from CharmRare_List import *
CharmRareList = CharmRareList()
skimOutputUdst('CharmRare_Standalone', CharmRareList)
summaryOfLists(CharmRareList)

process(analysis_main)

print(statistics)
