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

from vertex import *

from variables import variables
import sys

use_central_database("development")
set_random_seed('#BAADF00D')

if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
    sys.exit(0)
inputFile = os.path.join(os.environ['BELLE2_VALIDATION_DATA_DIR'], 'analysis/mdst9_BGx1_b2jpsiks.root')
inputMdst('default', inputFile)


fillParticleList('pi+:all', '')
fillParticleList('mu+:all', '')

reconstructDecay('K_S0:pipi -> pi+:all pi-:all', 'dM<0.25')
reconstructDecay('J/psi:mumu -> mu+:all mu-:all', 'dM<0.11')
reconstructDecay('B0:jpsiks -> J/psi:mumu K_S0:pipi', 'Mbc > 5.27 and abs(deltaE)<0.2')

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
]

analysis_main.add_module('VariablesToNtuple',
                         treeName='B0TreeFit',
                         particleList='B0:jpsiks',
                         variables=variables,
                         fileName='TreeFitted_B0ToJPsiKs.root')

summaryOfLists(['B0:jpsiks'])

process(analysis_main)

print(statistics)
