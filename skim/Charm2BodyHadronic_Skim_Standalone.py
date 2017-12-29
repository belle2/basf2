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


set_log_level(LogLevel.INFO)
gb2_setuprel = 'build-2017-10-16'
import sys
import os
import glob

ccbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_00051*_prod00000198_task0000051*.root']


inputMdstList('default', fileList)


loadStdCharged()

from Charm2BodyHadronic_List import *


DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm()
skimOutputUdst('Charm2BodyHadronic', DstToD0PiD0ToHpJmList)
summaryOfLists(DstToD0PiD0ToHpJmList)


process(analysis_main)

print(statistics)
