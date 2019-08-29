#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for dielectron + missing energy searches
# Giacomo De Pietro 2019 (giacomo.depietro@roma3.infn.it)
#
#######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-02-00'

dielectron_path = Path()

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=dielectron_path)

stdE('all', path=dielectron_path)

skimCode = encodeSkimName('DielectronPlusMissingEnergy')

from skim import dark
dielectron_list = dark.DielectronPlusMissingEnergyList(path=dielectron_path)

skimOutputUdst(skimCode, dielectron_list, path=dielectron_path)
summaryOfLists(dielectron_list, path=dielectron_path)

setSkimLogging(path=dielectron_path)
process(dielectron_path)

print(statistics)
