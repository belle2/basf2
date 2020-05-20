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
from skim.lowMulti import TwoTrackLeptonsForLuminosity

path = b2.Path()
skim = TwoTrackLeptonsForLuminosity(prescale=1, OutputFileName="../TwoTrackMuonsForLuminosity.udst.root")

ma.inputMdst("default", "../TwoTrackMuonsForLuminosity.dst.root", path=path)
skim(path)
b2.process(path)
