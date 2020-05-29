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
from skim.lowMulti import LowMassTwoTrack

path = b2.Path()
skim = LowMassTwoTrack(OutputFileName="../LowMassTwoTrackPions.udst.root")

ma.inputMdst("default", "../LowMassTwoTrackPions.dst.root", path=path)
skim(path)
b2.process(path)
