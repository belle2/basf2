#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../BottomoniumPiPiUpsilon.dst.root</input>
  <output>../BottomoniumPiPiUpsilon.udst.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
  <interval>nightly</interval>
</header>
"""

__author__ = "S. Spataro && S. Jia"

import basf2 as b2
import modularAnalysis as ma
from skim.quarkonium import BottomoniumPiPiUpsilon

path = b2.Path()

fileList = ['../BottomoniumPiPiUpsilon.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = BottomoniumPiPiUpsilon(OutputFileName='../BottomoniumPiPiUpsilon.udst.root')
skim(path)
b2.process(path)
