#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../PRsemileptonicUntagged.udst.root</input>
    <output>PRsemileptonicUntagged_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables

path = Path()

inputMdst('default', '../PRsemileptonicUntagged.udst.root', path=path)

cutAndCopyLists('B0:all', ['B0:L1', 'B0:L2'], '', path=path)

buildRestOfEvent('B0:all', path=path)
appendROEMask('B0:all', 'basic',
              'pt>0.05 and -2<dr<2 and -4.0<dz<4.0',
              'E>0.05',
              path=path)
buildContinuumSuppression('B0:all', 'basic', path=path)

variables.addAlias('d0_p', 'daughter(0, p)')
variables.addAlias('d1_p', 'daughter(1, p)')
variables.addAlias('MissM2', 'WE_MissM2(basic,0)')

variablesToHistogram(
    filename='PRsemileptonicUntagged_Validation.root',
    decayString='B0:all',
    variables=[
        ('Mbc', 100, 4.0, 5.3),
        ('d0_p', 100, 0, 5.2),  # Pion momentum
        ('d1_p', 100, 0, 5.2),   # Lepton momentum
        ('MissM2', 100, -5, 5)
    ],
    variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3)],
    path=path)

process(path)
print(statistics)
