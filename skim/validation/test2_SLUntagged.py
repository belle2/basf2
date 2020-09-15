#!/usr/bin/env python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../SLUntagged.udst.root</input>
    <output>SLUntagged_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
from validation_tools.metadata import create_validation_histograms

path = b2.Path()

ma.inputMdst('default', '../SLUntagged.udst.root', path=path)

ma.cutAndCopyLists('B+:all', ['B+:SLUntagged_0', 'B+:SLUntagged_1', 'B+:SLUntagged_2', 'B+:SLUntagged_3'], '', path=path)

ma.buildRestOfEvent('B+:all', path=path)
ma.appendROEMask('B+:all', 'basic',
                 'pt>0.05 and -2<dr<2 and -4.0<dz<4.0',
                 'E>0.05',
                 path=path)
ma.buildContinuumSuppression('B+:all', 'basic', path=path)

vm.addAlias('d1_p', 'daughter(1,p)')
vm.addAlias('MissM2', 'weMissM2(basic,0)')

histogramFilename = 'SLUntagged_Validation.root'
myEmail = 'Phil Grace <philip.grace@adelaide.edu.au>'

create_validation_histograms(
    rootfile=histogramFilename,
    particlelist='B+:all',
    variables_1d=[
        ('cosThetaBetweenParticleAndNominalB', 100, -6.0, 4.0, 'cosThetaBY', myEmail, '', ''),
        ('Mbc', 100, 4.0, 5.3, 'Mbc', myEmail, '', ''),
        ('d1_p', 100, 0, 5.2, 'Signal-side lepton momentum', myEmail, '', ''),
        ('MissM2', 100, -5, 5, 'Missing mass squared', myEmail, '', '')
    ],
    variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3, 'Mbc vs deltaE', myEmail, '', '')],
    path=path)


b2.process(path)
print(b2.statistics)
