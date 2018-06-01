#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#######################################################
#
# Skims for LFV Z'
# Ilya Komarov (ilya.komarov@desy.de), 2018
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from skimExpertFunctions import encodeSkimName, setSkimLogging


set_log_level(LogLevel.INFO)
gb2_setuprel = 'build-2018-05-04'

import sys
import os
import glob


if len(sys.argv) > 1:
    bkgType = sys.argv[1]
    f = open('inputFiles/' + bkgType + '.txt', 'r')
    fileList = f.readline().replace("\n", "")
    f.close()
    print('Running over file ' + fileList)
elif len(sys.argv) == 1:
    fileList = [
        '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900520000_0.root',
        '/group/belle2/users/jbennett/release-01-00-02/4S/signal/3900420000_*.root'
    ]
    bkgType = 'old'


if len(sys.argv) > 1:
    inputMdstList('default', glob.glob(fileList))
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)

loadStdCharged()

from LFVZpInvisible_List import *
SysList = LFVZpInvisibleList()
skimCode = encodeSkimName('LFVZpInvisible')
skimOutputUdst(skimCode + bkgType, SysList)
summaryOfLists(SysList)

setSkimLogging()
process(analysis_main)

print(statistics)
