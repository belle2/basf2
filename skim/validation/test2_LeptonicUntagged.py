#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../LeptonicUntagged.udst.root</input>
    <output>LeptonicUntagged_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables
from validation_tools.metadata import create_validation_histograms

path = Path()

inputMdst('default', '../LeptonicUntagged.udst.root', path=path)

cutAndCopyLists('B-:all', ['B-:L0', 'B-:L1'], '', path=path)

buildRestOfEvent('B-:all', path=path)
appendROEMask('B-:all', 'basic',
              'pt>0.05 and -2<dr<2 and -4.0<dz<4.0',
              'E>0.05',
              path=path)
buildContinuumSuppression('B-:all', 'basic', path=path)

variables.addAlias('d0_p', 'daughter(0,p)')
variables.addAlias('d0_electronID', 'daughter(0,electronID)')
variables.addAlias('d0_muonID', 'daughter(0,muonID)')
variables.addAlias('MissP', 'weMissP(basic,0)')

histogramFilename = 'LeptonicUntagged_Validation.root'
myEmail = 'Phil Grace <philip.grace@adelaide.edu.au>'

create_validation_histograms(
    rootfile=histogramFilename,
    particlelist='B-:all',
    variables_1d=[
        ('Mbc', 100, 4.0, 5.3, 'Mbc', myEmail, '', ''),
        ('d0_p', 100, 0, 5.2, 'Signal-side lepton momentum', myEmail, '', ''),
        ('d0_electronID', 100, 0, 1, 'electronID of signal-side lepton', myEmail, '', ''),
        ('d0_muonID', 100, 0, 1, 'electronID of signal-side lepton', myEmail, '', ''),
        ('R2', 100, 0, 1, 'R2', myEmail, '', ''),
        ('MissP', 100, 0, 5.3, 'Missing momentum of event (CMS frame)', myEmail, '', '')
    ],
    variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3, 'Mbc vs deltaE', myEmail, '', '')],
    path=path)

process(path)
print(statistics)
