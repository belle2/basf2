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

reset_database()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)

set_log_level(LogLevel.INFO)

ccbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/' +
     'prod00000972/s00/e0000/4S/r00000/ccbar/sub00/*root']

ccbar_noBG = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/' +
     'prod00000973/s00/e0000/4S/r00000/ccbar/sub00/*root']

mixed_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/' +
     'prod00000962/s00/e0000/4S/r00000/mixed/sub00/*root']

charged_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/' +
     'prod00000964/s00/e0000/4S/r00000/charged/sub00/*root']

uubar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/' +
     'prod00000966/s00/e0000/4S/r00000/uubar/sub00/*root']

ddbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/' +
     'prod00000968/s00/e0000/4S/r00000/ddbar/sub00/*root']

ssbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/' +
     'prod00000970/s00/e0000/4S/r00000/ssbar/sub00/*root']

filelist = ccbar_noBG
inputMdstList('default', filelist)

loadStdCharged()
loadStdKS()
loadStdLoosePi0()

from Charm3BodyHadronic_List import *

# D0ToHpJmPi0List = D0ToHpJmPi0()
# skimOutputUdst('D0ToHpJmPi0_Standalone_ccbar_wBG', D0ToHpJmPi0List)
# summaryOfLists(D0ToHpJmPi0List)

DstToD0PiD0ToHpJmPi0List = DstToD0PiD0ToHpJmPi0()
skimOutputUdst('DstToD0PiD0ToHpJmPi0_Standalone_ccbar_noBG', DstToD0PiD0ToHpJmPi0List)
summaryOfLists(DstToD0PiD0ToHpJmPi0List)

# DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs()
# skimOutputUdst('DstToD0PiD0ToHpHmKs_Standalone_mixed_wBG', DstToD0PiD0ToHpHmKsList)
# summaryOfLists(DstToD0PiD0ToHpHmKsList)

process(analysis_main)

print(statistics)
