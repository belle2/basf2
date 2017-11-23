#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#

# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdV0s import *
from stdPi0s import *
gb2_setuprel = 'build-2017-10-16'

ccbar_wBG = \
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-07-00/DBxxxxxxxx/MC6/prod00000198/s00/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_00051*_prod00000198_task0000051*.root']


inputMdstList('default', fileList)


loadStdCharged()
loadStdKS()
loadStdSkimPi0()

from Charm2BodyNeutrals_List import *


DstList = DstToD0Neutrals()
skimOutputUdst('Charm2BodyNeutrals', DstList)
summaryOfLists(DstList)

process(analysis_main)

print(statistics)
