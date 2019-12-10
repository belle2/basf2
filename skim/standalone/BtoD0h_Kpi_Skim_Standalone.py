#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# B+ -> anti-D0 (-> Kpi) h+ skims
# Author: Niharika Rout
#
#######################################################

from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK
import skimExpertFunctions as sef

b2.set_log_level(b2.LogLevel.INFO)
b2.gb2_setuprel = 'release-04-00-04'

path = b2.create_path()

skimCode = sef.encodeSkimName('BtoD0h_Kpi')

fileList = sef.get_test_file("MC12_mixedBGx1")

ma.inputMdstList('default', fileList, path=path)

# create and fill pion and kaon ParticleLists
stdPi('all', path=path)
stdK('all', path=path)

ma.applyCuts(list_name='pi+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)
ma.applyCuts(list_name='K+:all', cut='abs(dr) < 2 and abs(dz) < 5', path=path)

# B+ to D(->h+h-)h+ Skim
from skim.btocharm import BsigToD0hToKpiList
from skim.standardlists.charm import loadStdD0_Kpi

loadStdD0_Kpi(path=path)
BtoD0h_Kpi_list = BsigToD0hToKpiList(path=path)
sef.skimOutputUdst(skimCode, BtoD0h_Kpi_list, path=path)
ma.summaryOfLists(BtoD0h_Kpi_list, path=path)

sef.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
