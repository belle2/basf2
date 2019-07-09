#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>17240100.udst.root</input>
    <output>Charm2BodyHadronic_Validation.root</output>
    <contact>gonggd@mail.ustc.edu.cn</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK


c2bhdpath = Path()

inputMdst('default', '17240100.udst.root', path=c2bhdpath)

# Load particle lists
stdPi('loose', path=c2bhdpath)
stdK('loose', path=c2bhdpath)
stdPi('all', path=c2bhdpath)
stdK('all', path=c2bhdpath)

reconstructDecay('D0:HpJm0_test -> pi+:loose K-:loose', '1.80 < M < 1.93 and useCMSFrame(p)>2.2', path=c2bhdpath)
reconstructDecay('D*+:HpJm0_test -> D0:HpJm0_test pi+:all', '0 < Q < 0.018', path=c2bhdpath)

from variables import variables
variables.addAlias('M_D0', 'daughter(0,InvM)')
variables.addAlias('Pcms_D0', 'daughter(0,useCMSFrame(p))')
variables.addAlias('d0_spi', 'daughter(1,d0)')
variables.addAlias('z0_spi', 'daughter(1,z0)')
variables.addAlias('dr_spi', 'daughter(1,dr)')
variables.addAlias('dz_spi', 'daughter(1,dz)')
variables.addAlias('Pcms_spi', 'daughter(1,useCMSFrame(p))')
variables.addAlias('Pcms_Dst', 'useCMSFrame(p)')
variablesToHistogram(
    filename='Charm2BodyHadronic_Validation.root',
    decayString='D*+:HpJm0_test',
    variables=[
                    ('M_D0', 100, 1.80, 1.),
                    ('Pcms_D0', 100, 2, 6),
                    ('d0_spi', 100, -1.2, 1.2),
                    ('z0_spi', 100, -3.3, 3.3),
                    ('dr_spi', 100, -1.2, 1.2),
                    ('dz_spi', 100, -3.3, 3.3),
                    ('Pcms_spi', 100, 0, 0.8),
                    ('Pcms_Dst', 100, 2, 6),
                    ('Q', 100, 0, 0.018),
    ],
    path=c2bhdpath)

process(c2bhdpath)
print(statistics)
