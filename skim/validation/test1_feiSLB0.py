#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../feiSLB0.dst.root</input>
    <output>../feiSLB0.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

import basf2 as b2
import modularAnalysis as ma
from skim.fei import feiSLB0

path = b2.Path()

fileList = ['../feiSLB0.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = feiSLB0(OutputFileName='../feiSLB0.udst.root')
skim(path)
b2.process(path)
