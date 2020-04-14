#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TwoTrackPions.dst.root</input>
  <output>../TwoTrackPions.udst.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from skim import dark

b2.set_log_level(b2.LogLevel.INFO)

TwoTrackPions_path = b2.Path()

ma.inputMdst('default', '../TwoTrackPions.dst.root', path=TwoTrackPions_path)

TwoTrackPions_list = dark.TwoTrackPionsList(path=TwoTrackPions_path)
expert.skimOutputUdst('../TwoTrackPions.udst.root', TwoTrackPions_list, path=TwoTrackPions_path)

ma.summaryOfLists(TwoTrackPions_list, path=TwoTrackPions_path)

expert.setSkimLogging(path=TwoTrackPions_path)

b2.process(TwoTrackPions_path)

print(b2.statistics)
