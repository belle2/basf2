#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../LowMassTwoTrackPions.dst.root</input>
  <output>../LowMassTwoTrackPions.udst.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from skim import dark

b2.set_log_level(b2.LogLevel.INFO)

LowMassTwoTrackPions_path = b2.Path()

ma.inputMdst('default', '../LowMassTwoTrackPions.dst.root', path=LowMassTwoTrackPions_path)

LowMassTwoTrackPions_list = dark.LowMassTwoTrackList(path=LowMassTwoTrackPions_path)
expert.skimOutputUdst('../LowMassTwoTrackPions.udst.root', LowMassTwoTrackPions_list, path=LowMassTwoTrackPions_path)

ma.summaryOfLists(LowMassTwoTrackPions_list, path=LowMassTwoTrackPions_path)

expert.setSkimLogging(path=LowMassTwoTrackPions_path)

b2.process(LowMassTwoTrackPions_path)

print(b2.statistics)
