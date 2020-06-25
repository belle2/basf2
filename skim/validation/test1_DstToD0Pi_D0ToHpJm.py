#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../DstToD0Pi_D0ToHpJm.dst.root</input>
    <output>17240100.udst.root</output>
    <contact>gonggd@mail.ustc.edu.cn</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "G. GONG"

import basf2 as b2
import modularAnalysis as ma
from skim.charm import DstToD0Pi_D0ToHpJm

path = b2.Path()

fileList = ['../DstToD0Pi_D0ToHpJm.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = DstToD0Pi_D0ToHpJm()
skim(path)
b2.process(path)
