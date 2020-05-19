#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoDh_Kshh.dst.root</input>
    <output>14140200.udst.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = [
    "N. Dash",
    "N. Rout"
]

import basf2 as b2
import modularAnalysis as ma
from skim.btocharm import BtoD0h_Kshh

path = b2.Path()

fileList = ['../BtoDh_Kshh.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = BtoD0h_Kshh()
skim(path)
b2.process(path)
