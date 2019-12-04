#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for dielectron + missing energy searches
# Giacomo De Pietro 2019 (giacomo.depietro@roma3.infn.it)
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
import stdCharged as charged
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

dielectron_path = b2.Path()

fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=dielectron_path)

charged.stdE('all', path=dielectron_path)

skimCode = expert.encodeSkimName('DielectronPlusMissingEnergy')

from skim import dark
dielectron_list = dark.DielectronPlusMissingEnergyList(path=dielectron_path)

expert.skimOutputUdst(skimCode, dielectron_list, path=dielectron_path)
ma.summaryOfLists(dielectron_list, path=dielectron_path)

expert.setSkimLogging(path=dielectron_path)
b2.process(dielectron_path)

print(b2.statistics)
