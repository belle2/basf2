#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for e+e-, mu+mu-, and pi+pi-
# Xing-Yu Zhou 2020 (zhouxy@buaa.edu.cn)
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from skim import dark

b2.set_log_level(b2.LogLevel.INFO)

TwoTrackLeptonsForLuminosity_path = b2.Path()

fileList = expert.get_test_file("MC13_mixedBGx1")
ma.inputMdstList('default', fileList, path=TwoTrackLeptonsForLuminosity_path)

TwoTrackLeptonsForLuminosity_code = expert.encodeSkimName('TwoTrackLeptonsForLuminosity')
TwoTrackLeptonsForLuminosity_list = dark.TwoTrackLeptonsForLuminosityList(path=TwoTrackLeptonsForLuminosity_path)
expert.skimOutputUdst(TwoTrackLeptonsForLuminosity_code, TwoTrackLeptonsForLuminosity_list, path=TwoTrackLeptonsForLuminosity_path)

ma.summaryOfLists(TwoTrackLeptonsForLuminosity_list, path=TwoTrackLeptonsForLuminosity_path)

expert.setSkimLogging(path=TwoTrackLeptonsForLuminosity_path)

b2.process(TwoTrackLeptonsForLuminosity_path)

print(b2.statistics)
