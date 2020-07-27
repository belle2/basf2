#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../SLUntagged.dst.root</input>
    <output>../SLUntagged.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

import basf2 as b2
import modularAnalysis as ma
from skim.semileptonic import SLUntagged

path = b2.Path()

fileList = ['../SLUntagged.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = SLUntagged(OutputFileName='../SLUntagged.udst.root')
skim(path)
b2.process(path)
