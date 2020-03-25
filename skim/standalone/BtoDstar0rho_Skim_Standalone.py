#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#####################################################################
#
# B+ -> anti-D0 (-> Kpi, Kpipipi, Kpipi0, Kspipi) rho+,
#       anti-D*0( -> D0(-> Kpi, Kpipipi, Kpipi0, Kspipi) pi0) rho+ skim
# Fernando Abudinen
#
#####################################################################
from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons, loadStdAllRhoPlus
import skimExpertFunctions as sef

b2.set_log_level(b2.LogLevel.INFO)
b2.gb2_setuprel = 'release-04-01-01'

path = b2.create_path()

skimCode = sef.encodeSkimName('BtoDstar0rho')

fileList = sef.get_test_file("MC12_mixedBGx1")

ma.inputMdstList('default', fileList, path=path)

stdPi('all', path=path)
stdK('all', path=path)
loadStdPi0ForBToHadrons(path=path)
loadStdAllRhoPlus(path=path)

ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)
ma.applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)

# B+ -> anti-D0 rho+, anti-D*0( -> D0 pi0) rho+
from skim.btocharm import BtoDstar0rhoList
import skim.standardlists.charm as ssc

ssc.loadStdD0_Kpi(path=path)
ssc.loadStdD0_Kpipipi(path=path)
ssc.loadStdD0_Kpipi0(path=path)
ssc.loadStdDstar0_D0pi0_Kpi(path=path)
ssc.loadStdDstar0_D0pi0_Kpipipi(path=path)
ssc.loadStdDstar0_D0pi0_Kpipi0(path=path)
BtoDstar0rho_list = BtoDstar0rhoList(path=path)
sef.skimOutputUdst(skimCode, BtoDstar0rho_list, path=path)
ma.summaryOfLists(BtoDstar0rho_list, path=path)


sef.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
