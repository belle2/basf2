#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Particle gun muon events for EKLM, full reconstruction (all detectors).

import os
from basf2 import *
from simulation import *
from reconstruction import *

set_log_level(LogLevel.WARNING)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [13, -13],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [1., 3.],
    'thetaGeneration': 'uniform',
    'thetaParams': [17., 150.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}

pGun.param(param_pGun)

output = register_module('RootOutput')
output.param('outputFileName', 'ParticleGunMuonsFull.root')

# Create paths
main = create_path()
main.add_module(eventinfosetter)
main.add_module(pGun)
add_simulation(main)
add_reconstruction(main)
main.add_module(output)

process(main)
print(statistics)
