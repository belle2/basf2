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
variables.addAlias('d0_electronID', 'daughter(0,electronID)')
variables.addAlias('d0_muonID', 'daughter(0,muonID)')

variablesToHistogram(
    filename='LeptonicUntagged_Validation.root',
    decayString='B-:all',
    variables=[
        ('Mbc', 100, 4.0, 5.3),
        ('d0_p', 100, 0, 5.2),  # Lepton momentum
        ('d0_electronID', 100, 0, 1),
        ('d0_muonID', 100, 0, 1)
    ],
    variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3)],
    path=path)

process(path)
print(statistics)
