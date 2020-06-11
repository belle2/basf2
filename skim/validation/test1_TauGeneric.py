#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TauGeneric.dst.root</input>
  <output>../TauGeneric.udst.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

__author__ = "Kenji Inami"


import basf2 as b2
import modularAnalysis as ma
from skim.taupair import TauGeneric

path = b2.Path()

fileList = ['../TauGeneric.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = TauGeneric(OutputFileName='../TauGeneric.udst.root')
skim(path)
b2.process(path)
