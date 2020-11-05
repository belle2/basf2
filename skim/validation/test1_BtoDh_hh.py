#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoDh_hh.dst.root</input>
    <output>14140100.udst.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "N. Rout"

import basf2 as b2
import modularAnalysis as ma
from skim.btocharm import BtoD0h_hh

path = b2.Path()

fileList = ['../BtoDh_hh.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = BtoDh_hh()
skim(path)
b2.process(path)
