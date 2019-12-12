#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# neutral B to charm mode:
# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> k+ pi-
#
# Chiara La Licata 2019/Oct/18
#
######################################################

from ROOT import Belle2
from basf2 import *
import modularAnalysis as ma
from stdCharged import stdPi, stdK
from stdPi0s import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-04'

mypath = Path()

skimCode = encodeSkimName('B0toDStarPi_D0pi_kpi')

fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=mypath)

# create and fill pion and kaon ParticleLists
stdPi('all', path=mypath)
stdK('all', path=mypath)
stdPi0s('skim', path=mypath)

ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5',
             path=mypath)
ma.applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5',
             path=mypath)

# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> k+ pi-
from skim.btocharm import loadB0toDstarh_kpi
from skim.standardlists.charm import loadStdD0_Kpi, loadStdDstarPlus_D0pi_Kpi
loadStdD0_Kpi(path=mypath)
loadStdDstarPlus_D0pi_Kpi(path=mypath)
B0toDstarhList_kpi = loadB0toDstarh_kpi(path=mypath)

skimOutputUdst(skimCode, B0toDstarhList_kpi, path=mypath)
summaryOfLists(B0toDstarhList_kpi, path=mypath)

setSkimLogging(mypath)
process(mypath)

# print out the summary
print(statistics)
