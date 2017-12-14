#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################
#
#
#   Partial Reconstruction of D* in charm B decays
#
#  L. Cremaldi, R. Godang, R.Cheaib
#
#
#
################################

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

gb2_setuprel = 'build-2017-10-16'


fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]
inputMdstList('default', fileList)

loadStdCharged()
# PR Skim
from PRsemileptonicUntagged_List import *
PRList = PRList()
skimOutputUdst('PRsemileptonicUntagged_' + bkgType, PRList)
summaryOfLists(PRList)
process(analysis_main)
# print out the summary
print(statistics)
