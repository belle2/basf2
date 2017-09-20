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
from stdPi0s import *

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
    bkgType = 'ccbarRel7'

if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)
loadStdSkimPi0()
loadStdCharged()
stdPi0s('loose')
stdPhotons('loose')
loadStdKS()


from Charm3BodyHadronic_List import *

# D0ToHpJmPi0List = D0ToHpJmPi0()
# skimOutputUdst('D0ToHpJmPi0_Standalone_ccbar_wBG', D0ToHpJmPi0List)
# summaryOfLists(D0ToHpJmPi0List)

DstToD0PiD0ToHpJmPi0List = DstToD0PiD0ToHpJmPi0()
skimOutputUdst('outputFiles/Charm3BodyHadronic_' + bkgType, DstToD0PiD0ToHpJmPi0List)
summaryOfLists(DstToD0PiD0ToHpJmPi0List)

# DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs()
# skimOutputUdst('DstToD0PiD0ToHpHmKs_Standalone_mixed_wBG', DstToD0PiD0ToHpHmKsList)
# summaryOfLists(DstToD0PiD0ToHpHmKsList)

process(analysis_main)

print(statistics)
