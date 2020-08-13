#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# 100 pure bkg events are produced
########################################################

"""
<header>
<output>ECLBkgOutput.root</output>
<contact>Elisa Manoni, elisa.manoni@pg.infn.it</contact>
</header>
"""

import os
from basf2 import *
import glob
from simulation import add_simulation
from reconstruction import add_reconstruction

# Create paths
main = create_path()

# Event setting and info
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

# pGun is used as artifact, otherwise FullSim will complain that no MCParticles Array is available
pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [2212],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.0],
    'thetaGeneration': 'fixed',
    'thetaParams': [0.],
    'phiGeneration': 'fixed',
    'phiParams': [0., ],
    'vertexGeneration': 'fixed',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}
pGun.param(param_pGun)
main.add_module(pGun)

# Fixed random seed
set_random_seed(123456)

# bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
else:
    print('Warning: variable BELLE2_BACKGROUND_DIR is not set')
B2INFO('Using background samples from ' + os.environ['BELLE2_BACKGROUND_DIR'])

add_simulation(main, bkgfiles=bg)
add_reconstruction(main)

# eclDataAnalysis module
ecldataanalysis = register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', '../ECLBkgOutput.root')
ecldataanalysis.param('doTracking', 1)
main.add_module(ecldataanalysis)

process(main)
# print(statistics)
