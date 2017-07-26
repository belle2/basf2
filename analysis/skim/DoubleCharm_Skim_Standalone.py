#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Double charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *

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
        sys.exit('Could not find root file : ' + fileList[:-1])
    print('Running over file ' + fileList[:-1])
elif len(sys.argv) == 1:
    fileList = \
        ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
         'mdst_000001_prod00000001_task00000001.root'

         ]
    bkgType = 'old'


if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)
loadStdAllPi0()  # for stdCharm.py
loadStdCharged()
loadStdKS()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

# Double Charm Skim
from DoubleCharm_List import *
DCList = DoubleCharmList()
skimOutputUdst('outputFiles/DoubleCharm_' + bkgType, DCList)
summaryOfLists(DCList)

# make sure memory consumption is limited by restricting the amount of
# candidates to be reconstructed
for module in analysis_main.modules():
    if module.type() == "ParticleCombiner":
        print("Adjusting maximum of candidates for " + module.name())
        module.param("maximumNumberOfCandidates", 1000)

process(analysis_main)

# print out the summary
print(statistics)
