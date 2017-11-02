#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Semileptonic skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *

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
    fileList = ''
    bkgType = 'old'

if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)

loadStdCharged()

# Leptonic Skim
from LeptonicUntagged_List import *

lepList = LeptonicList()
skimOutputUdst('outputFiles/LeptonicUntagged_' + bkgType, lepList)
summaryOfLists(lepList)

process(analysis_main)

# print out the summary
print(statistics)
