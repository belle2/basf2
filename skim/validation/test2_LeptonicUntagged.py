#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../LeptonicUntagged.udst.root</input>
    <output>LeptonicUntagged_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from skim.leptonic import LeptonicUntagged

path = b2.Path()
skim = LeptonicUntagged()

ma.inputMdst('default', '../LeptonicUntagged.udst.root', path=path)
skim.validation_histograms(path)
b2.process(path)
