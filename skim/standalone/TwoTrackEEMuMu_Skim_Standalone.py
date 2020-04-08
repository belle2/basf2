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

TwoTrackEEMuMu_path = b2.Path()

fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=TwoTrackEEMuMu_path)

TwoTrackEEMuMu_code = expert.encodeSkimName('TwoTrackEEMuMu')
TwoTrackEEMuMu_list = dark.TwoTrackEEMuMuList(path=TwoTrackEEMuMu_path)
expert.skimOutputUdst(TwoTrackEEMuMu_code, TwoTrackEEMuMu_list, path=TwoTrackEEMuMu_path)

ma.summaryOfLists(TwoTrackEEMuMu_list, path=TwoTrackEEMuMu_path)

expert.setSkimLogging(path=TwoTrackEEMuMu_path)

b2.process(TwoTrackEEMuMu_path)

print(b2.statistics)
