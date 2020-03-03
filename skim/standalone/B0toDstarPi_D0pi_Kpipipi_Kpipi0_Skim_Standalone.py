#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# neutral B to charm mode:
#
# B0 -> D*(D0 pi+)pi+
# combination of two D0 modes:
# with D0 -> k- pi+ pi0 and D0 -> k- pi+ pi+ pi-
#
# Chiara La Licata 2020/Feb/04
#
######################################################

from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file, skimOutputUdst

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-01-01'

mypath = b2.Path()

skimCode = encodeSkimName('B0toDstarPi_D0pi_Kpipipi_Kpipi0')

fileList = get_test_file("MC12_mixedBGx1")

ma.inputMdstList('default', fileList, path=mypath)

# create and fill pion and kaon ParticleLists
stdPi('all', path=mypath)
stdK('all', path=mypath)
loadStdPi0ForBToHadrons(path=mypath)


ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5',
             path=mypath)
ma.applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5',
             path=mypath)

# B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> K+ pi- pi0 and D0 -> k- pi+ pi+ pi-
from skim.btocharm import loadB0toDstarPi_Kpipi0, loadB0toDstarPi_Kpipipi
from skim.standardlists.charm import loadStdD0_Kpipi0, loadStdDstarPlus_D0pi_Kpipi0
from skim.standardlists.charm import loadStdD0_Kpipipi, loadStdDstarPlus_D0pi_Kpipipi
loadStdD0_Kpipi0(path=mypath)
loadStdDstarPlus_D0pi_Kpipi0(path=mypath)
B0toDstarPiList_Kpipi0 = loadB0toDstarPi_Kpipi0(path=mypath)
loadStdD0_Kpipipi(path=mypath)
loadStdDstarPlus_D0pi_Kpipipi(path=mypath)
B0toDstarPiList_Kpipipi = loadB0toDstarPi_Kpipipi(path=mypath)

B0toDstarPiList = []

for chID, channel in enumerate(B0toDstarPiList_Kpipi0):
    B0toDstarPiList.append(channel)
for chID, channel in enumerate(B0toDstarPiList_Kpipipi):
    B0toDstarPiList.append(channel)

skimOutputUdst(skimCode, B0toDstarPiList, path=mypath)
ma.summaryOfLists(B0toDstarPiList, path=mypath)

setSkimLogging(mypath)
b2.process(mypath)

# print out the summary
print(b2.statistics)
