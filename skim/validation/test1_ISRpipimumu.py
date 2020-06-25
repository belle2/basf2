#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../ISRpipimumu.dst.root</input>
    <output>../ISRpipimumu.udst.root</output>
    <contact>jiasen@buaa.edu.cn</contact>
    <interval>nightly</interval>
</header>
"""

__author__ = "S. Jia"

import basf2 as b2
import modularAnalysis as ma
from skim.quarkonium import ISRpipicc

path = b2.Path()

fileList = ['../ISRpipimumu.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = ISRpipicc(OutputFileName='../ISRpipimumu.udst.root')
skim(path)
b2.process(path)
