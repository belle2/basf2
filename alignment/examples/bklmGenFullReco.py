#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
from reconstruction import add_reconstruction
import glob

main = create_path()

# specify number of events (500) to be generated in job
eventInfoSetter = register_module('EventInfoSetter')
eventInfoSetter.param('expList', [1])
eventInfoSetter.param('runList', [1])
eventInfoSetter.param('evtNumList', [100000])
main.add_module(eventInfoSetter)

# Generate single-track events with muons.
# Particle gun to shoot particles in the detector.
# Choose the particles you want to simulate
pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [13, -13],
    'nTracks': 1,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.5, 4.0],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [20., 160.],
    'phiGeneration': 'uniform',
    'phiParams': [0.0, 360.0],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}
pGun.param(param_pGun)
main.add_module(pGun)

# geometry parameter database
gearbox = register_module('Gearbox')
main.add_module(gearbox)
# detector geometry
geometry = register_module('Geometry')
# geometry.param({
#     "excludedComponents": ["MagneticField"],
#     "additionalComponents": ["MagneticField2d"],
# })
main.add_module(geometry)
# simulation
add_simulation(main)
# reconstruction
add_reconstruction(main, pruneTracks=False)

"""
for module in main.modules():
  if module.name() == 'Ext':
    module.param('Cosmic', True)
"""
output = register_module('RootOutput')
output.param('outputFileName', 'fullreco.root')
main.add_module(output)
main.add_module('Progress')
process(main)

# Print call statistics
print(statistics)
