#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for dimuon + missing energy searches
# Giacomo De Pietro 2019 (giacomo.depietro@roma3.infn.it)
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
import stdCharged as charged
import skimExpertFunctions as skimExp

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

dimuon_path = b2.Path()

fileList = skimExp.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=dimuon_path)

charged.stdMu('all', path=dimuon_path)

skimCode = skimExp.encodeSkimName('DimuonPlusMissingEnergy')

from skim import dark
dimuon_list = dark.DimuonPlusMissingEnergyList(path=dimuon_path)

skimExp.skimOutputUdst(skimCode, dimuon_list, path=dimuon_path)
skimExp.summaryOfLists(dimuon_list, path=dimuon_path)

skimExp.setSkimLogging(path=dimuon_path)
b2.process(dimuon_path)

print(b2.statistics)
