#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../BtoXll.dst.root</input>
  <output>../BtoXll.udst.root</output>
  <contact>soumen.halder@desy.de</contact>
</header>
"""


import basf2 as b2
import modularAnalysis as ma
from skim.ewp import BtoXll

path = b2.Path()

fileList = ['../BtoXll.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = BtoXll(OutputFileName='../BtoXll.udst.root')
skim(path)
b2.process(path)
