#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B -> D(*) h(*) skims
# P. Urquijo, 6/Jan/2015
#
######################################################
from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *

set_log_level(LogLevel.INFO)


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
    bkgType = 'notSpecified'


if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)


loadStdCharged()
loadStdPi0()
loadStdKS()

fillParticleList('pi+:all', '')
fillParticleList('K+:all', '')

# B- to D(->Kshh)h- Skim
from BtoDh_Kshh_List import *


loadD()
BtoDhList = BsigToDhToKshhList()
skimOutputUdst('outputFiles/BtoDh_Kshh_' + bkgType, BtoDhList)
summaryOfLists(BtoDhList)

process(analysis_main)

# print out the summary
print(statistics)
