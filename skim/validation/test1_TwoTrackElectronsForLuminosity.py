#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TwoTrackElectronsForLuminosity.dst.root</input>
  <output>../TwoTrackElectronsForLuminosity.udst.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from skim.lowMulti import TwoTrackLeptonsForLuminosity

path = b2.Path()
skim = TwoTrackLeptonsForLuminosity(prescale=1, OutputFileName="../TwoTrackElectronsForLuminosity.udst.root")

ma.inputMdst("default", "../TwoTrackElectronsForLuminosity.dst.root", path=path)
skim(path)
b2.process(path)
