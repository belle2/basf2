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
from skim import lowMulti

b2.set_log_level(b2.LogLevel.INFO)

LowMassTwoTrack_path = b2.Path()

fileList = expert.get_test_file("MC13_mixedBGx1")
ma.inputMdstList('default', fileList, path=LowMassTwoTrack_path)

LowMassTwoTrack_code = expert.encodeSkimName('LowMassTwoTrack')
LowMassTwoTrack_list = lowMulti.LowMassTwoTrackList(path=LowMassTwoTrack_path)
expert.skimOutputUdst(LowMassTwoTrack_code, LowMassTwoTrack_list, path=LowMassTwoTrack_path)

ma.summaryOfLists(LowMassTwoTrack_list, path=LowMassTwoTrack_path)

expert.setSkimLogging(path=LowMassTwoTrack_path)

b2.process(LowMassTwoTrack_path)

print(b2.statistics)
