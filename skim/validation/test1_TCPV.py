#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../TCPV.dst.root</input>
  <output>../TCPV.udst.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""


import basf2 as b2
import modularAnalysis as ma
from skim.tdcpv import TDCPV

path = b2.Path()

fileList = ['../TCPV.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = TDCPV(OutputFileName='../TCPV.udst.root')
skim(path)
b2.process(path)
