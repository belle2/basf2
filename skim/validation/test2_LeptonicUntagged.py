#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../LeptonicUntagged.udst.root</input>
    <output>LeptonicUntagged_Validation.root</output>
    <contact>hannah.wakeling@mail.mcgill.ca, philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables

path = Path()

inputMdst('default', '../LeptonicUntagged.udst.root', path=path)

cutAndCopyLists('B-:all', ['B-:L0', 'B-:L1'], '', path=path)

variables.addAlias('d0_p', 'daughter(0,p)')

variablesToHistogram(
    filename='LeptonicUntagged_Validation.root',
    decayString='B-:all',
    variables=[
        ('Mbc', 100, 4.0, 5.3),
        ('d0_p', 100, 0, 5.2)],  # Lepton momentum
    path=path)

process(path)
print(statistics)
