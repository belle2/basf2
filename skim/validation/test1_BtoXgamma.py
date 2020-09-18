#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoXgamma.dst.root</input>
    <output>12160100.udst.root</output>
    <contact>simon.wehle@desy.de</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "Simon Wehle"

import basf2 as b2
import modularAnalysis as ma
from skim.ewp import BtoXgamma


path = b2.Path()

fileList = ['../BtoXgamma.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = BtoXgamma()
skim(path)
b2.process(path)
