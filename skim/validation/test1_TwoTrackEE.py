#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TwoTrackEE.dst.root</input>
  <output>../TwoTrackEE.udst.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from skim import dark

b2.set_log_level(b2.LogLevel.INFO)

TwoTrackEEMuMu_path = b2.Path()

ma.inputMdst('default', '../TwoTrackEE.dst.root', path=TwoTrackEEMuMu_path)

TwoTrackEEMuMu_list = dark.TwoTrackEEMuMuList(path=TwoTrackEEMuMu_path)
expert.skimOutputUdst('../TwoTrackEE.udst.root', TwoTrackEEMuMu_list, path=TwoTrackEEMuMu_path)

ma.summaryOfLists(TwoTrackEEMuMu_list, path=TwoTrackEEMuMu_path)

expert.setSkimLogging(path=TwoTrackEEMuMu_path)

b2.process(TwoTrackEEMuMu_path)

print(b2.statistics)
