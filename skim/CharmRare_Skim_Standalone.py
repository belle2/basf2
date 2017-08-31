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

set_log_level(LogLevel.INFO)
import sys
import os
import glob

if len(sys.argv) > 1:
    bkgType = sys.argv[1]
    f = open('inputFiles/' + bkgType + '.txt', 'r')
    fileList = f.read()
    f.close()
    if not os.path.isfile(fileList[:-1]):
        sys.exit('Could not find root file: ' + fileList[:-1])
    print('Running over file ' + fileList[:-1])
elif len(sys.argv) == 1:
    fileList = \
        ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
         'mdst_0005*_prod00000198_task000005*.root'
         ]
    bkgType = 'ccbarOld'

if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)


loadStdSkimPi0()
loadStdSkimPhoton()
stdLooseMu()
stdLooseE()
from CharmRare_List import *
CharmRareList = CharmRareList()
skimOutputUdst('outputFiles/CharmRare_' + bkgType, CharmRareList)
summaryOfLists(CharmRareList)

process(analysis_main)

print(statistics)
