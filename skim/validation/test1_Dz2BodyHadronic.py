#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>./WG6_DstToD0pi_D0ToKpi.dst.root</input>
  <output>./WG6_DstToD0ToKpi.udst.root</output>
  <contact>lilongke@ihep.ac.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from skim.charm import DstToD0Pi_D0ToHpJm

path = b2.Path()

fileList = ["../WG6_DstToD0pi_D0ToKpi.dst.root"]
ma.inputMdstList('default', fileList, path=path)

skim = DstToD0Pi_D0ToHpJm(OutputFileName='../WG6_DstToD0ToKpi.udst.root')
skim(path)
b2.process(path)
