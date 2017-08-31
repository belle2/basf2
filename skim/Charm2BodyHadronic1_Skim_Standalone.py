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
from stdFSParticles import *

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

stdFSParticles()
loadStdCharged()

from Charm2BodyHadronic_List import *

D0ToHpJmList = D0ToHpJm()
skimOutputUdst('D0ToHpJm_Standalone_ccbar_noBG', D0ToHpJmList)
summaryOfLists(D0ToHpJmList)

DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm()
skimOutputUdst('DstToD0PiD0ToHpJm_Standalone_ccbar_noBG', DstToD0PiD0ToHpJmList)
summaryOfLists(DstToD0PiD0ToHpJmList)


process(analysis_main)

print(statistics)
