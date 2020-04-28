#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../LeptonicUntagged.dst.root</input>
    <output>../LeptonicUntagged.udst.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = "P. Grace"

import basf2 as b2
import modularAnalysis as ma
from skim.leptonic import LeptonicUntagged

path = b2.Path()
skim = LeptonicUntagged(OutputFileName="../LeptonicUntagged")

ma.inputMdst('default', '../LeptonicUntagged.dst.root', path=path)
skim(path)
b2.process(path)
