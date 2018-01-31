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
from stdV0s import *
from stdPi0s import *

gb2_setuprel = 'release-01-00-00'
set_log_level(LogLevel.INFO)

import sys
import os
import glob

fileList = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_00051*_prod00000198_task0000051*.root']

inputMdstList('default', fileList)


loadStdSkimPi0()
stdKshorts()
loadStdCharged()


from Charm3BodyHadronicD0_List import *
D0ToHpJmPi0List = D0ToHpJmPi0()
skimOutputUdst('Charm3BodyHadronicD0', D0ToHpJmPi0List)

summaryOfLists(D0ToHpJmPi0List)

process(analysis_main)

print(statistics)
