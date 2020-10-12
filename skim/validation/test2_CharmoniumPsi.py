#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
  <input>../CharmoniumPsi.udst.root</input>
  <output>CharmoniumPsi_Validation.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdMu, stdE
from validation_tools.metadata import create_validation_histograms

liason = 'Sen Jia (jiasen@buaa.edu.cn)'

# create a new path
path = b2.Path()

filelist = ['../CharmoniumPsi.udst.root']
ma.inputMdstList('default', filelist, path=path)

# use standard final state particle lists
stdE('loosepid', path=path)
stdMu('loosepid', path=path)

# [Y(3S) -> pi+pi- [Y(1S,2S) -> mu+mu-]] decay
ma.reconstructDecay('J/psi:mumu_test -> mu+:loosepid mu-:loosepid', '', path=path)
ma.reconstructDecay('J/psi:ee_test -> e+:loosepid e-:loosepid', '', path=path)
ma.copyList('J/psi:ll', 'J/psi:mumu_test', path=path)
ma.copyList('J/psi:ll', 'J/psi:ee_test', path=path)

# Print histograms.
create_validation_histograms(
    rootfile='CharmoniumPsi_Validation.root',
    particlelist='J/psi:ll',
    variables_1d=[(
        'InvM', 65, 2.7, 4.0,
        'J/#psi mass',
        liason,
        'J/psi mass',
        'J/psi peak is seen.',
        'M [GeV/c^{2}]', 'Events / (20 MeV/c^{2})',
        'shifter'
    )],
    path=path
)

b2.process(path)
print(b2.statistics)
