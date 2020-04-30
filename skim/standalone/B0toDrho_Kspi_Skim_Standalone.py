#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#####################################################################
#
# B0 -> D- (-> Kspi) rho+ skim
# Fernando Abudinen
#
#####################################################################
from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons, loadStdAllRhoPlus
import skimExpertFunctions as sef

b2.set_log_level(b2.LogLevel.INFO)
b2.gb2_setuprel = 'release-04-01-01'

path = b2.create_path()

skimCode = sef.encodeSkimName('B0toDrho_Kspi')

fileList = sef.get_test_file("MC12_mixedBGx1")

ma.inputMdstList('default', fileList, path=path)

stdPi('all', path=path)
stdKshorts(path=path)
loadStdPi0ForBToHadrons(path=path)
loadStdAllRhoPlus(path=path)

ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)

# B0 -> D- (-> Kspi) rho+
from skim.btocharm import B0toDrho_KspiList
from skim.standardlists.charm import loadStdDplus_Kspi

loadStdDplus_Kspi(path=path)
B0toDrho_Kspi_list = B0toDrho_KspiList(path=path)
sef.skimOutputUdst(skimCode, B0toDrho_Kspi_list, path=path)
ma.summaryOfLists(B0toDrho_Kspi_list, path=path)


sef.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
