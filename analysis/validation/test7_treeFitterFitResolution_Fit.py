#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>TreeFitter_Fitted_B0ToJPsiKs.root</output>
  <contact>Jo-Frederik Krohn; jo-frederik.krohn@desy.de</contact>
  <description>Reconstruct B0sig->J/PsiKs using the TreeFitter. Use the ..._Plot.py to plot the resolutions.  </description>
</header>
"""

from basf2 import *
from modularAnalysis import *
from modularAnalysis import analysis_main
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay

from vertex import *
from vertex import vertexTree

from variables import variables
import sys

# Get Special GT for the flavor tagger weight files
use_central_database("development")


if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
    sys.exit(0)
inputFile = os.path.join(os.environ['BELLE2_VALIDATION_DATA_DIR'], 'analysis/mdst9_BGx1_b2jpsiks.root')
inputMdst('default', inputFile)


# Reconstruction of signal side and MC match
fillParticleList('pi+:all', '')
fillParticleList('mu+:all', '')

reconstructDecay('K_S0:pipi -> pi+:all pi-:all', 'dM<0.25')
reconstructDecay('J/psi:mumu -> mu+:all mu-:all', 'dM<0.11')
reconstructDecay('B0:jpsiks -> J/psi:mumu K_S0:pipi', 'Mbc > 5.2 and abs(deltaE)<0.2')

matchMCTruth('B0:jpsiks')

vertexTree(
    list_name='B0:jpsiks',
    conf_level=-1,
    massConstraint=[],
    ipConstraintDim=0,
    updateAllDaughters=True,
    path=analysis_main,
)

variables = [
    'isSignal',  # B0
    'M',
    'InvM',
    'chiProb',

    'p',
    'mcP',
    'pErr',
    'E',
    'mcE',
    'E_uncertainty',

    'x',
    'y',
    'z',
    'x_uncertainty',
    'y_uncertainty',
    'z_uncertainty',
    'mcDX',
    'mcDY',
    'mcDZ',

    'daughter(0,x)',  # J/Psi
    'daughter(0,y)',
    'daughter(0,z)',
    'daughter(0,z_uncertainty)',
    'daughter(0,y_uncertainty)',
    'daughter(0,x_uncertainty)',
    'daughter(0,mcDX)',
    'daughter(0,mcDY)',
    'daughter(0,mcDX)',

    'daughter(0,p)',
    'daughter(0,mcP)',
    'daughter(0,pErr)',
    'daughter(0,E)',
    'daughter(0,mcE)',
    'daughter(0,E_uncertainty)',
    'daughter(0,M)',
    'daughter(0,InvM)',

    'daughter(1,x)',  # Ks
    'daughter(1,y)',
    'daughter(1,z)',
    'daughter(1,z_uncertainty)',
    'daughter(1,y_uncertainty)',
    'daughter(1,x_uncertainty)',
    'daughter(1,mcDX)',
    'daughter(1,mcDY)',
    'daughter(1,mcDX)',

    'daughter(1,p)',
    'daughter(1,mcP)',
    'daughter(1,pErr)',
    'daughter(1,E)',
    'daughter(1,mcE)',
    'daughter(1,E_uncertainty)',
    'daughter(1,M)',
    'daughter(1,InvM)',
]

analysis_main.add_module('VariablesToNtuple',
                         treeName='B0TreeFit',
                         particleList='B0:jpsiks',
                         variables=variables,
                         fileName='TreeFitted_B0ToJPsiKs.root')

summaryOfLists(['B0:jpsiks'])

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
