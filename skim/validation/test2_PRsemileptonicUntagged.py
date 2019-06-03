#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../PRsemileptonic.udst.root</input>
    <output>PRsemileptonicUntagged_Validation.root</output>
    <contact>hannah.wakeling@mail.mcgill.ca, philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables

path = Path()

inputMdst('default', '../PRsemileptonic.udst.root', path=path)

cutAndCopyLists('B0:all', ['B0:L1', 'B0:L2'], '', path=path)

variables.addAlias('d1_p', 'daughter(1, p)')

variablesToHistogram(
    filename='PRsemileptonicUntagged_Validation.root',
    decayString='B0:all',
    variables=[
        ('Mbc', 100, 4.0, 5.3),
        ('d0_p', 100, 0, 5.2),  # Pion momentum
        ('d1_p', 100, 0, 5.2)   # Lepton momentum
    ],
    variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3)],
    path=path)

process(path)
print(statistics)
