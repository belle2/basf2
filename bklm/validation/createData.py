#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to simulate 1000 charged-muon single-track events   #
#    using the ParticleGun for BKLM validation. (No             #
#    background hits overlaid.)                                 #
#                                                               #
#    written by Leo Piilonen, VT                                #
#    piilonen@vt.edu                                            #
#                                                               #
#################################################################

"""
<header>
    <output>muon-BKLMValidation.root</output>
    <contact>piilonen@vt.edu</contact>
    <description>Create events with 1 muon track for BKLM validation.</description>
</header>
"""

import glob
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_random_seed(981543)

output_filename = '../muon-BKLMValidation.root'

print(output_filename)

path = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [7])
eventinfosetter.param('runList', [35])
eventinfosetter.param('evtNumList', [1000])
path.add_module(eventinfosetter)

progress = register_module('Progress')
path.add_module(progress)

pgun = register_module('ParticleGun')
param_pgun = {
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.5, 5.0],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [35., 130.],
    'phiGeneration': 'uniform',
    'phiParams': [0.0, 360.0],
    'vertexGeneration': 'fixed',
    'xVertexParams': [0.0],
    'yVertexParams': [0.0],
    'zVertexParams': [0.0],
}
pgun.param(param_pgun)
path.add_module(pgun)

# Need to define this to replace default (non-physical) magnetic field with 2D map
components = [
    'MagneticField2d',
    'COIL',
    'STR',
    'BeamPipe',
    'Cryostat',
    'HeavyMetalShield',
    'VXDService',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'ECL',
    'BKLM',
    'EKLM',
]

if 'BELLE2_BACKGROUND_DIR' in os.environ:
    background_files = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
    print('Background files:')
    print(background_files)
    add_simulation(path, components, background_files)
else:
    print('Warning: variable BELLE2_BACKGROUND_DIR is not set')
    add_simulation(path, components)

add_reconstruction(path, components)

output = register_module('RootOutput')
output.param('outputFileName', output_filename)
output.param('branchNames', ['MCParticles', 'ExtHits', 'Muids', 'BKLMHit2ds', 'EKLMHit2ds'])
path.add_module(output)

process(path)
print(statistics)
