#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro, 25/Jul/2016
#
######################################################

from basf2 import *
from modularAnalysis import *

set_log_level(LogLevel.ERROR)

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
    fileList = [
        '/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/prod00000962/s00/e0000/4S/r00000/mixed/sub00/' +
        'mdst_001724_prod00000962_task00001729.root']
    bkgType = 'notSpecified'

if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)


# Bottomonium Skim
from Bottomonium_List import *
YList = EtabList()
skimOutputUdst('outputFiles/Bottomonium1_' + bkgType, YList)
summaryOfLists(YList)
process(analysis_main)

# print out the summary
print(statistics)
