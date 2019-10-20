#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>17240100.udst.root</input>
    <output>DstToD0Pi_D0ToHpJm_Validation.root</output>
    <contact>gonggd@mail.ustc.edu.cn</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from variables import variables
from validation_tools.metadata import create_validation_histograms


c2bhdpath = Path()

inputMdst('default', '17240100.udst.root', path=c2bhdpath)

# Load particle lists
stdPi('loose', path=c2bhdpath)
stdK('loose', path=c2bhdpath)
stdPi('all', path=c2bhdpath)
stdK('all', path=c2bhdpath)

reconstructDecay('D0:HpJm0_test -> pi+:loose K-:loose', '1.80 < M < 1.93 and useCMSFrame(p)>2.2', path=c2bhdpath)
reconstructDecay('D*+:HpJm0_test -> D0:HpJm0_test pi+:all', '0 < Q < 0.018', path=c2bhdpath)

variables.addAlias('M_D0', 'daughter(0,InvM)')
variables.addAlias('Pcms_D0', 'daughter(0,useCMSFrame(p))')
variables.addAlias('d0_spi', 'daughter(1,d0)')
variables.addAlias('z0_spi', 'daughter(1,z0)')
variables.addAlias('dr_spi', 'daughter(1,dr)')
variables.addAlias('dz_spi', 'daughter(1,dz)')
variables.addAlias('Pcms_spi', 'daughter(1,useCMSFrame(p))')
variables.addAlias('Pcms_Dst', 'useCMSFrame(p)')


histogramFilename = 'DstToD0Pi_D0ToHpJm_Validation.root'
myEmail = 'Guanda Gong <gonggd@mail.ustc.edu.cn>'


create_validation_histograms(
    rootfile=histogramFilename,
    particlelist='D*+:HpJm0_test',
    variables_1d=[
                    ('M_D0', 100, 1.80, 1., 'Mass distribution of $D^{0}$', myEmail,
                     '', '', 'M(D^{0}) [GeV/c^{2}]', 'shifter'),
                    ('Pcms_D0', 100, 2, 6, 'momentum of $D_{0}$ in CMS Frame', myEmail,
                     '', '', '$P_{cms}(D^{0}) [GeV/c^{2}]', 'shifter'),
                    ('d0_spi', 100, -1.2, 1.2, 'd0 of slow pi', myEmail,
                     '', '', 'd0_spi [cm]', 'shifter'),
                    ('z0_spi', 100, -3.3, 3.3, 'z0 of slow pi', myEmail,
                     '', '', 'z0_spi [cm]', 'shifter'),
                    ('dr_spi', 100, -1.2, 1.2, 'dr of slow pi', myEmail,
                     '', 'dr_spi [cm]', 'shifter'),
                    ('dz_spi', 100, -3.3, 3.3, 'dz of slow pi', myEmail,
                     '', 'dz_spi [cm]', 'shifter'),
                    ('Pcms_spi', 100, 0, 0.8, 'momentum of slow pi in CMS Frame', myEmail,
                     '', 'P_{cms}(#pi_{s}) [GeV/c]', 'shifter'),
                    ('Pcms_Dst', 100, 2, 6, 'momentum of $D_{*}$ in CMS Frame', myEmail,
                     '', 'P_{cms}(D*) {GeV/c}', 'shifter'),
                    ('Q', 100, 0, 0.018, 'Released energy in $D^{*}$ decay', myEmail,
                     '', 'Q [GeV]', 'shifter'),
    ],
    path=c2bhdpath)

process(c2bhdpath)
print(statistics)
