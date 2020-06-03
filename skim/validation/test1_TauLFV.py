#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TauLFV.dst.root</input>
  <output>../TauLFV.udst.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

__author__ = "Kenji Inami"


import basf2 as b2
import modularAnalysis as ma
from skim.taupair import TauLFV

path = b2.Path()

fileList = ['../TauLFV.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = TauLFV(OutputFileName='../TauLFV.udst.root')
skim(path)
b2.process(path)
