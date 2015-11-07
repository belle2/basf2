#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
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
# main.add_module('Cosmics')
# geometry parameter database
gearbox = register_module('Gearbox')
main.add_module(gearbox)
# detector geometry
geometry = register_module('Geometry')
geometry.param({
    "excludedComponents": ["MagneticField"],
    "additionalComponents": ["MagneticField2d"],
})
main.add_module(geometry)
# detector simulation
g4sim = register_module('FullSim')
main.add_module(g4sim)
# digitization in simulator
pxd_digitizer = register_module('PXDDigitizer')
main.add_module(pxd_digitizer)
pxd_clusterizer = register_module('PXDClusterizer')
main.add_module(pxd_clusterizer)
svd_digitizer = register_module('SVDDigitizer')
main.add_module(svd_digitizer)
svd_clusterizer = register_module('SVDClusterizer')
main.add_module(svd_clusterizer)
cdc_digitizer = register_module('CDCDigitizer')
main.add_module(cdc_digitizer)
top_digitizer = register_module('TOPDigitizer')
main.add_module(top_digitizer)
arich_digitizer = register_module('ARICHDigitizer')
main.add_module(arich_digitizer)
ecl_digitizer = register_module('ECLDigitizer')
main.add_module(ecl_digitizer)
bklm_digitizer = register_module('BKLMDigitizer')
main.add_module(bklm_digitizer)
eklm_digitizer = register_module('EKLMDigitizer')
main.add_module(eklm_digitizer)

# reconstruction
add_mc_reconstruction(main)
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
