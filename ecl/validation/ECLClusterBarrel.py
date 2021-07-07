#!/usr/bin/env python3
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
<output>ECLClusterOutputBarrel.root</output>
<contact>Elisa Manoni, elisa.manoni@pg.infn.it</contact>
</header>
"""

import os
import glob
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

# Create paths
main = b2.create_path()

# Event setting and info
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

# Fixed random seed
b2.set_random_seed(123456)

# single particle generator settings
pGun = b2.register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.1],
    'thetaGeneration': 'uniform',
    'thetaParams': [33., 130.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}

pGun.param(param_pGun)
main.add_module(pGun)

# bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
else:
    print('Warning: variable BELLE2_BACKGROUND_DIR is not set')
b2.B2INFO('Using background samples from ' + os.environ['BELLE2_BACKGROUND_DIR'])

add_simulation(main, bkgfiles=bg)
add_reconstruction(main)

# eclDataAnalysis module
ecldataanalysis = b2.register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', '../ECLClusterOutputBarrel.root')
ecldataanalysis.param('doTracking', 1)
ecldataanalysis.param('doDigits', 1)
main.add_module(ecldataanalysis)

b2.process(main)
# print(statistics)
