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
import skimExpertFunctions as skimExp

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

dielectron_path = b2.Path()

fileList = skimExp.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=dielectron_path)

charged.stdE('all', path=dielectron_path)

skimCode = skimExp.encodeSkimName('DielectronPlusMissingEnergy')

from skim import dark
dielectron_list = dark.DielectronPlusMissingEnergyList(path=dielectron_path)

skimExp.skimOutputUdst(skimCode, dielectron_list, path=dielectron_path)
skimExp.summaryOfLists(dielectron_list, path=dielectron_path)

skimExp.setSkimLogging(path=dielectron_path)
b2.process(dielectron_path)

print(b2.statistics)
