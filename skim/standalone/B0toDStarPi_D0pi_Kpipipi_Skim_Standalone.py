#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# neutral B to charm mode:
# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> K- pi+ pi- pi+
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

skimCode = encodeSkimName('B0toDStarPi_D0pi_Kpipipi')

fileList = get_test_file("MC12_mixedBGx1")

inputMdstList('default', fileList, path=mypath)

# create and fill pion and kaon ParticleLists
stdPi('all', path=mypath)
stdK('all', path=mypath)
stdPi0s('skim', path=mypath)

ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5',
             path=mypath)
ma.applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5',
             path=mypath)


# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> K- pi+ pi- pi+ Skim
from skim.btocharm import loadB0toDstarPi_Kpipipi
from skim.standardlists.charm import loadStdD0_Kpipipi, loadStdDstarPlus_D0pi_Kpipipi
loadStdD0_Kpipipi(path=mypath)
loadStdDstarPlus_D0pi_Kpipipi(path=mypath)
B0toDstarPiList_Kpipipi = loadB0toDstarPi_Kpipipi(path=mypath)

skimOutputUdst(skimCode, B0toDstarPiList_Kpipipi, path=mypath)
summaryOfLists(B0toDstarPiList_Kpipipi, path=mypath)

setSkimLogging(mypath)
process(mypath)

# print out the summary
print(statistics)
