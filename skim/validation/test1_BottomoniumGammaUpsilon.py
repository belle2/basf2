#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../BottomoniumGammaUpsilon.dst.root</input>
  <output>../BottomoniumGammaUpsilon.udst.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
  <interval>nightly</interval>
</header>
"""

__author__ = "S. Spataro && S. Jia"

import basf2 as b2
import modularAnalysis as ma
from skim.quarkonium import BottomoniumUpsilon

path = b2.Path()

fileList = ['../BottomoniumGammaUpsilon.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = BottomoniumUpsilon(OutputFileName='../BottomoniumGammaUpsilon.udst.root')
skim(path)
b2.process(path)
