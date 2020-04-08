#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TwoTrackPiPi.dst.root</input>
  <output>../TwoTrackPiPi.udst.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from skim import dark

b2.set_log_level(b2.LogLevel.INFO)

TwoTrackPiPi_path = b2.Path()

ma.inputMdst('default', '../TwoTrackPiPi.dst.root', path=TwoTrackPiPi_path)

TwoTrackPiPi_list = dark.TwoTrackPiPiList(path=TwoTrackPiPi_path)
expert.skimOutputUdst('../TwoTrackPiPi.udst.root', TwoTrackPiPi_list, path=TwoTrackPiPi_path)

ma.summaryOfLists(TwoTrackPiPi_list, path=TwoTrackPiPi_path)

expert.setSkimLogging(path=TwoTrackPiPi_path)

b2.process(TwoTrackPiPi_path)

print(b2.statistics)
