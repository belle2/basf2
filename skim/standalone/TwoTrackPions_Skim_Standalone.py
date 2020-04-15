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

TwoTrackPions_path = b2.Path()

fileList = expert.get_test_file("MC13_mixedBGx1")
ma.inputMdstList('default', fileList, path=TwoTrackPions_path)

TwoTrackPions_code = expert.encodeSkimName('TwoTrackPions')
TwoTrackPions_list = dark.TwoTrackPionsList(path=TwoTrackPions_path)
expert.skimOutputUdst(TwoTrackPions_code, TwoTrackPions_list, path=TwoTrackPions_path)

ma.summaryOfLists(TwoTrackPions_list, path=TwoTrackPions_path)

expert.setSkimLogging(path=TwoTrackPions_path)

b2.process(TwoTrackPions_path)

print(b2.statistics)
