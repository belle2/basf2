#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../feiSLBplus.dst.root</input>
    <output>../feiSLBplus.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

import basf2 as b2
import modularAnalysis as ma
from skim.fei import feiSLBplus

path = b2.Path()

fileList = ['../feiSLBplus.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = feiSLBplus(OutputFileName='../feiSLBplus.udst.root')
skim(path)
b2.process(path)
