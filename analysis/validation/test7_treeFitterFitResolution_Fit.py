#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>../TreeFitted_B0ToJPsiKs.root</output>
  <contact>Jo-Frederik Krohn; jkrohn@student.unimelb.edu.au</contact>
  <interval>nightly</interval>
</header>
"""

# Reconstruct B0 to J/PsiKs using the TreeFitter. Use the ..._Plot.py
# to plot the resolutions.

from basf2 import create_path, process, statistics, set_random_seed

from modularAnalysis import inputMdst, reconstructDecay, fillParticleList, matchMCTruth
import os
from vertex import treeFit

import sys

path = create_path()
set_random_seed('#BAADF00D')

if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
    sys.exit(0)
inputFile = os.path.join(os.environ['BELLE2_VALIDATION_DATA_DIR'], 'analysis/prerel04_eph3_BGx1_b2jpsiks.root')
inputMdst('default', inputFile, path=path)


fillParticleList('pi+:all', '', path=path)
fillParticleList('mu+:all', '', path=path)

reconstructDecay('K_S0:pipi -> pi+:all pi-:all', 'dM<0.25', path=path)
reconstructDecay('J/psi:mumu -> mu+:all mu-:all', 'dM<0.11', path=path)
reconstructDecay('B0:jpsiks -> J/psi:mumu K_S0:pipi', 'Mbc > 5.27 and abs(deltaE)<0.2', path=path)

matchMCTruth('B0:jpsiks', path=path)

treeFit(
    list_name='B0:jpsiks',
    conf_level=-1,
    updateAllDaughters=True,
    path=path,
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
    'mcDecayVertexX',
    'mcDecayVertexY',
    'mcDecayVertexZ',
]

path.add_module('VariablesToNtuple',
                treeName='B0TreeFit',
                particleList='B0:jpsiks',
                variables=variables,
                fileName='../TreeFitted_B0ToJPsiKs.root')

process(path)

print(statistics)
