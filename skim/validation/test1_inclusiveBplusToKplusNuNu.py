#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../inclusiveBplusToKplusNuNu.dst.root</input>
    <output>12160400.udst.root</output>
    <contact>cyrille.praz@desy.de</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = [
    'Cyrille Praz',
]

import basf2 as b2
import modularAnalysis as ma
from skim.ewp import inclusiveBplusToKplusNuNu

path = b2.Path()

fileList = ['../inclusiveBplusToKplusNuNu.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = inclusiveBplusToKplusNuNu()
skim(path)
b2.process(path)
