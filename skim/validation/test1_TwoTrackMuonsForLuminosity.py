#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TwoTrackMuonsForLuminosity.dst.root</input>
  <output>../TwoTrackMuonsForLuminosity.udst.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from skim import dark

b2.set_log_level(b2.LogLevel.INFO)

TwoTrackLeptonsForLuminosity_path = b2.Path()

ma.inputMdst('default', '../TwoTrackMuonsForLuminosity.dst.root', path=TwoTrackLeptonsForLuminosity_path)

TwoTrackLeptonsForLuminosity_list = dark.TwoTrackLeptonsForLuminosityList(path=TwoTrackLeptonsForLuminosity_path)
expert.skimOutputUdst(
    '../TwoTrackMuonsForLuminosity.udst.root',
    TwoTrackLeptonsForLuminosity_list,
    path=TwoTrackLeptonsForLuminosity_path)

ma.summaryOfLists(TwoTrackLeptonsForLuminosity_list, path=TwoTrackLeptonsForLuminosity_path)

expert.setSkimLogging(path=TwoTrackLeptonsForLuminosity_path)

b2.process(TwoTrackLeptonsForLuminosity_path)

print(b2.statistics)
