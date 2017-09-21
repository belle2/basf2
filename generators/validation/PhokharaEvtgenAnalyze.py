#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>PhokharaEvtgenData.root</input>
    <output>PhokharaEvtgenAnalysis.root</output>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Analysis of e+ e- -> J/psi eta_c events.</description>
</header>
"""

from basf2 import *
from modularAnalysis import *

inputMdst('None', 'PhokharaEvtgenData.root')

fillParticleListFromMC('J/psi:MC', '')
fillParticleListFromMC('eta_c:MC', '')
reconstructDecay('gamma:MC -> J/psi:MC eta_c:MC', '')

tools = ['EventMetaData', '^gamma:MC']
tools += ['InvMass', '^gamma:MC']
tools += ['Kinematics', '^gamma:MC -> ^J/psi:MC eta_c:MC']
tools += ['CustomFloats[Ecms]', '^gamma:MC']

ntupleFile('PhokharaEvtgenAnalysis.root')
ntupleTree('tree', 'gamma:MC', tools)

process(analysis_main)

print(statistics)
