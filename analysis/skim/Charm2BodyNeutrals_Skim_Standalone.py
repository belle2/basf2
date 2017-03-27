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
from stdFSParticles import *
from stdV0s import *
from stdPi0s import *

reset_database()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)

set_log_level(LogLevel.INFO)

ccbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_00051*_prod00000198_task0000051*.root']

ccbar_noBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000194/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_00051*_prod00000194_task0000051*.root']

mixed_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000188/s00/e0000/4S/r00000/mixed/sub00/' +
     'mdst_00051*_prod00000188_task0000051*.root']

charged_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000189/s00/e0000/4S/r00000/charged/sub00/' +
     'mdst_00051*_prod00000189_task0000051*.root']

uubar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000195/s00/e0000/4S/r00000/uubar/sub00/' +
     'mdst_00051*_prod00000195_task0000051*.root']

ddbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000196/s00/e0000/4S/r00000/ddbar/sub00/' +
     'mdst_00051*_prod00000196_task0000051*.root']

ssbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000197/s00/e0000/4S/r00000/ssbar/sub00/' +
     'mdst_00051*_prod00000197_task0000051*.root']

filelist = ccbar_noBG
inputMdstList('default', filelist)

stdFSParticles()
loadStdCharged()
loadStdKS()
loadStdPi0()

from Charm2BodyNeutrals_List import *


D0ToNeutralsList = D0ToNeutralsList()
skimOutputUdst('D0ToNeutrals_Standalone_'+filelist, D0ToNeutralsList)
summaryOfLists(D0ToNeutralsList)

'''
DstToD0NeutralsList = DstToD0NeutralsList()
skimOutputUdst('DstToD0Neutrals_Standalone_'+filelist, DstToD0NeutralsList)
summaryOfLists(DstToD0NeutralsList)
'''

process(analysis_main)

print(statistics)
