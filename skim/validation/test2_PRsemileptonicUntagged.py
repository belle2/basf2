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
from validation_tools.metadata import create_validation_histograms

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

histogramFilename = 'PRsemileptonicUntagged_Validation.root'
myEmail = 'Phil Grace <philip.grace@adelaide.edu.au>'

create_validation_histograms(
    rootfile=histogramFilename,
    particlelist='B0:all',
    variables_1d=[
        ('Mbc', 100, 4.0, 5.3, 'Mbc', myEmail, '', ''),
        ('d0_p', 100, 0, 5.2, 'Signal-side pion momentum', myEmail, '', ''),
        ('d1_p', 100, 0, 5.2, 'Signal-side lepton momentum', myEmail, '', ''),
        ('MissM2', 100, -5, 5, 'Missing mass squared', myEmail, '', '')
    ],
    variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3, 'Mbc vs deltaE', myEmail, '', '')],
    path=path)

process(path)
print(statistics)
