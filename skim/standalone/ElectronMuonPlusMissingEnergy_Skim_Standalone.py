#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for electron-muon pair + missing energy searches
# Giacomo De Pietro 2019 (giacomo.depietro@roma3.infn.it)
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
import stdCharged as charged
import skimExpertFunctions as skimExp

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

emu_path = b2.Path()

fileList = skimExp.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=emu_path)

charged.stdE('all', path=emu_path)
charged.stdMu('all', path=emu_path)

skimCode = skimExp.encodeSkimName('ElectronMuonPlusMissingEnergy')

from skim import dark
emu_list = dark.ElectronMuonPlusMissingEnergyList(path=emu_path)

skimExp.skimOutputUdst(skimCode, emu_list, path=emu_path)
skimExp.summaryOfLists(emu_list, path=emu_path)

skimExp.setSkimLogging(path=emu_path)
b2.process(emu_path)

print(b2.statistics)
