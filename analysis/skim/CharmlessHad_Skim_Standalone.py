#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Had skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdLightMesons import *
from stdV0s import *
from stdPi0s import *
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


loadStdAllPi0()
stdPi0s('loose')
loadStdPhoton()
stdPhotons('loose')
loadStdCharged()
loadStdKS()
loadStdLightMesons()
loadStdSkimPi0()
# Had Skim
from CharmlessHad_List import *
HadList = CharmlessHadList()
skimOutputUdst('outputFiles/CharmlessHad_' + bkgType, HadList)
summaryOfLists(HadList)

process(analysis_main)

# print out the summary
print(statistics)
