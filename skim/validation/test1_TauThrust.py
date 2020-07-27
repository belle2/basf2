#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TauThrust.dst.root</input>
  <output>../TauThrust.udst.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

__author__ = "Kenji Inami"


import basf2 as b2
import modularAnalysis as ma
from skim.taupair import TauThrust

path = b2.Path()

fileList = ['../TauThrust.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = TauThrust(OutputFileName='../TauThrust.udst.root')
skim(path)
b2.process(path)
