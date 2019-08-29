#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for electron-muon pair + missing energy searches
# Giacomo De Pietro 2019 (giacomo.depietro@roma3.infn.it)
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

dimuon_path = Path()

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=dimuon_path)

stdE('all', path=dimuon_path)
stdMu('all', path=dimuon_path)

skimCode = encodeSkimName('ElectronMuonPlusMissingEnergy')

from skim import dark
dimuon_list = dark.ElectronMuonPlusMissingEnergyList(path=dimuon_path)

skimOutputUdst(skimCode, dimuon_list, path=dimuon_path)
summaryOfLists(dimuon_list, path=dimuon_path)

setSkimLogging(path=dimuon_path)
process(dimuon_path)

print(statistics)
