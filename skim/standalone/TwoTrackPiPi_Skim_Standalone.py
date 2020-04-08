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
gb2_setuprel = 'release-04-00-00'

TwoTrackPiPi_path = b2.Path()

fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=TwoTrackPiPi_path)

TwoTrackPiPi_code = expert.encodeSkimName('TwoTrackPiPi')
TwoTrackPiPi_list = dark.TwoTrackPiPiList(path=TwoTrackPiPi_path)
expert.skimOutputUdst(TwoTrackPiPi_code, TwoTrackPiPi_list, path=TwoTrackPiPi_path)

ma.summaryOfLists(TwoTrackPiPi_list, path=TwoTrackPiPi_path)

expert.setSkimLogging(path=TwoTrackPiPi_path)

b2.process(TwoTrackPiPi_path)

print(b2.statistics)
