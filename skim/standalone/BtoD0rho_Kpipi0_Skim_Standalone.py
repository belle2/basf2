#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################################
#
# B+ -> anti-D0 (-> Kpipi0) rho+, anti-D*0( -> D0(-> Kpipi0) pi0) rho+ skim
# Fernando Abudinen
#
#############################################################################
from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons, loadStdAllRhoPlus
import skimExpertFunctions as sef

b2.set_log_level(b2.LogLevel.INFO)
b2.gb2_setuprel = 'release-04-01-01'

path = b2.create_path()

skimCode = sef.encodeSkimName('BtoD0rho_Kpipi0')

fileList = sef.get_test_file("MC12_mixedBGx1")

ma.inputMdstList('default', fileList, path=path)

stdPi('all', path=path)
stdK('all', path=path)
loadStdPi0ForBToHadrons(path=path)
loadStdAllRhoPlus(path=path)

ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)
ma.applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)

# B+ -> anti-D0 (-> Kpipi0) rho+, anti-D*0( -> D0(-> Kpipi0) pi0) rho+
from skim.btocharm import BtoD0rho_Kpipi0List
from skim.standardlists.charm import loadStdD0_Kpipi0, loadStdDstar0_D0pi0_Kpipi0

loadStdD0_Kpipi0(path=path)
loadStdDstar0_D0pi0_Kpipi0(path=path)
BtoD0rho_Kpipi0_list = BtoD0rho_Kpipi0List(path=path)
sef.skimOutputUdst(skimCode, BtoD0rho_Kpipi0_list, path=path)
ma.summaryOfLists(BtoD0rho_Kpipi0_list, path=path)


sef.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
