#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../CharmoniumPsi.dst.root</input>
  <output>../CharmoniumPsi.udst.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
  <interval>nightly</interval>
</header>
"""

__author__ = "S. Spataro && S. Jia"

import basf2 as b2
import modularAnalysis as ma
from skim.quarkonium import CharmoniumPsi

path = b2.Path()

fileList = ['../CharmoniumPsi.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = CharmoniumPsi(OutputFileName='../CharmoniumPsi.udst.root')
skim(path)
b2.process(path)
