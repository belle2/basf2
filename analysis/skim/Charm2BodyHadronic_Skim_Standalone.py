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

reset_database()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)

set_log_level(LogLevel.INFO)

import sys
import os
import glob

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
if len(sys.argv) > 1:
    bkgType = sys.argv[1]
    f = open('inputFiles/' + bkgType + '.txt', 'r')
    fileList = f.read()
    f.close()
    if not os.path.isfile(fileList[:-1]):
        sys.exit('Could not find root file : ' + fileList[:-1])
    print('Running over file ' + fileList[:-1])
elif len(sys.argv) == 1:
    fileList = mixed_wBG
    bkgType = 'oldmixed'

if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)


loadStdCharged()

from Charm2BodyHadronic_List import *

DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm()
skimOutputUdst('outputFiles/Charm2BodyHadronic_' + bkgType, DstToD0PiD0ToHpJmList)
summaryOfLists(DstToD0PiD0ToHpJmList)


process(analysis_main)

print(statistics)
