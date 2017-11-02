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


if len(sys.argv) > 1:
    bkgType = sys.argv[1]
    f = open('inputFiles/' + bkgType + '.txt', 'r')
    fileList = f.read()
    f.close()
    if not os.path.isfile(fileList[:-1]):
        sys.exit('Could not find root file : ' + fileList[:-1])
    print('Running over file ' + fileList[:-1])
elif len(sys.argv) == 1:
    fileList = ccbar_wBG
    bkgType = 'ccbarOld'

if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)


# stdFSParticle()
loadStdCharged()

from Charm2BodyHadronicD0_List import *

D0ToHpJmList = D0ToHpJm()
skimOutputUdst('outputFiles/Charm2BodyHadronicD0_' + bkgType, D0ToHpJmList)
summaryOfLists(D0ToHpJmList)


process(analysis_main)

print(statistics)
