#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>17240100.udst.root</input>
    <output>DstToD0Pi_D0ToHpJm_Validation.root</output>
    <contact>gonggd@mail.ustc.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from variables import variables as vm
from validation_tools.metadata import create_validation_histograms


c2bhdpath = b2.Path()

ma.inputMdst('default', '17240100.udst.root', path=c2bhdpath)

# Load particle lists
stdPi('loose', path=c2bhdpath)
stdK('loose', path=c2bhdpath)
stdPi('all', path=c2bhdpath)
stdK('all', path=c2bhdpath)

ma.reconstructDecay('D0:HpJm0_test -> pi+:loose K-:loose', '1.80 < M < 1.93 and useCMSFrame(p)>2.2', path=c2bhdpath)
ma.reconstructDecay('D*+:HpJm0_test -> D0:HpJm0_test pi+:all', '0 < Q < 0.018', path=c2bhdpath)

vm.addAlias('M_D0', 'daughter(0,InvM)')
vm.addAlias('Pcms_D0', 'daughter(0,useCMSFrame(p))')
vm.addAlias('d0_spi', 'daughter(1,d0)')
vm.addAlias('z0_spi', 'daughter(1,z0)')
vm.addAlias('dr_spi', 'daughter(1,dr)')
vm.addAlias('dz_spi', 'daughter(1,dz)')
vm.addAlias('Pcms_spi', 'daughter(1,useCMSFrame(p))')
vm.addAlias('Pcms_Dst', 'useCMSFrame(p)')


histogramFilename = 'DstToD0Pi_D0ToHpJm_Validation.root'
myEmail = 'Guanda Gong <gonggd@mail.ustc.edu.cn>'


create_validation_histograms(
    rootfile=histogramFilename,
    particlelist='D*+:HpJm0_test',
    variables_1d=[
        ('M_D0', 100, 1.80, 1., 'Mass distribution of $D^{0}$', myEmail,
         'mass of D0 (mean=1.86483)', 'Please check agreement of: mean, sigma and ratio of signal and background',
         'M(D^{0}) [GeV/c^{2}]', 'shifter'),
        ('Pcms_D0', 100, 2, 6, 'momentum of $D_{0}$ in CMS Frame', myEmail,
         'CMS momentum of D0', 'Please check agreement of lineshape',
         '$P_{cms}(D^{0}) [GeV/c^{2}]', 'shifter'),
        ('d0_spi', 100, -1.2, 1.2, 'd0 of slow pi', myEmail,
         'd0 of slow pion', 'provided for the SVD and PXD group',
         'd0_spi [cm]', 'shifter'),
        ('z0_spi', 100, -3.3, 3.3, 'z0 of slow pi', myEmail,
         'z0 of slow pion', 'provided for the SVD and PXD group',
         'z0_spi [cm]', 'shifter'),
        ('dr_spi', 100, -1.2, 1.2, 'dr of slow pi', myEmail,
         'dr of slow pion', 'provided for the SVD and PXD group',
         'dr_spi [cm]', 'shifter'),
        ('dz_spi', 100, -3.3, 3.3, 'dz of slow pi', myEmail,
         'dz of slow pion', 'provided for the SVD and PXD group',
         'dz_spi [cm]', 'shifter'),
        ('Pcms_spi', 100, 0, 0.8, 'momentum of slow pi in CMS Frame', myEmail,
         'CMS momentum of slow pion', 'Please check agreement of lineshape',
         'P_{cms}(#pi_{s}) [GeV/c]', 'shifter'),
        ('Pcms_Dst', 100, 2, 6, 'momentum of $D_{*}$ in CMS Frame', myEmail,
         'CMS momentum of slow pion', 'Please check agreement of lineshape',
         'P_{cms}(D*) {GeV/c}', 'shifter'),
        ('Q', 100, 0, 0.018, 'Released energy in $D^{*}$ decay', myEmail,
         'Q = M(D0 pi) - M(D0) - M(pi), and it peaks around 0.006 GeV',
         'Please check agreement of: mean, sigma and ratio of signal and background',
         'Q [GeV]', 'shifter'),
    ],
    path=c2bhdpath)

b2.process(c2bhdpath)
print(b2.statistics)
