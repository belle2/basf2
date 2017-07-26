#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
# G. Casarosa, 7/Oct/2016

# deprecated
# decided to split the many charm skims in more than one file

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdPi0s import *
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


loadStdSkimPhoton()
loadStdSkimPi0()
stdLooseE()
stdLooseMu()
stdPi0s('loose')
from CharmRare_List import *
CharmRareList = CharmRareList()
skimOutputUdst('outputFiles/Charm_' + bkgType, CharmRareList)
summaryOfLists(CharmRareList)

process(analysis_main)

print(statistics)
