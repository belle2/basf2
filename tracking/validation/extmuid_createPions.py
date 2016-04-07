#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to simulate 1000 charged-pion single-track events   #
#    using the ParticleGun for ext and muid validation. (No     #
#    background hits overlaid.)                                 #
#                                                               #
#    written by Leo Piilonen, VT                                #
#    piilonen@vt.edu                                            #
#                                                               #
#################################################################

"""
<header>
    <output>pion-ExtMuidValidation.root</output>
    <contact>piilonen@vt.edu</contact>
    <description>Create events with 1 pion track for ext/muid validation.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_random_seed(654321)

output_filename = '../pion-ExtMuidValidation.root'

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
    'pdgCodes': [-211, 211],
    'nTracks': 1,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.5, 5.0],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [15., 150.],
    'phiGeneration': 'uniform',
    'phiParams': [0.0, 360.0],
    'vertexGeneration': 'fixed',
    'xVertexParams': [0.0],
    'yVertexParams': [0.0],
    'zVertexParams': [0.0],
}
pgun.param(param_pgun)
path.add_module(pgun)

add_simulation(path)
add_reconstruction(path)

output = register_module('RootOutput')
output.param('outputFileName', output_filename)
output.param('branchNames', ['MCParticles', 'ExtHits', 'Muids', 'BKLMHit2ds', 'EKLMHit2ds'])
path.add_module(output)

process(path)
print(statistics)
