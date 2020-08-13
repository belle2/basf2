#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../feiHadronicBplus.dst.root</input>
    <output>../feiHadronicBplus.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

import basf2 as b2
import modularAnalysis as ma
from skim.fei import feiHadronicBplus

path = b2.Path()

fileList = ['../feiHadronicBplus.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = feiHadronicBplus(OutputFileName='../feiHadronicBplus.udst.root')
skim(path)
b2.process(path)
