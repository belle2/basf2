#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../TDCP.dst.root</input>
  <output>../TDCP.udst.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""


import basf2 as b2
import modularAnalysis as ma
from skim.tcpv import TCPV

path = b2.Path()

fileList = ['../TCPV.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = TCPV(OutputFileName='../TCPV.udst.root')
skim(path)
b2.process(path)
