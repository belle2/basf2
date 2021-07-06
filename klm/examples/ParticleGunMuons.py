#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Particle gun muon events.

import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction

basf2.set_log_level(basf2.LogLevel.WARNING)

eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])

pGun = basf2.register_module('ParticleGun')
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

output = basf2.register_module('RootOutput')
output.param('outputFileName', 'ParticleGunMuons.root')

# Create path
main = basf2.create_path()
main.add_module(eventinfosetter)
main.add_module(pGun)
add_simulation(main)
add_reconstruction(main)
main.add_module(output)
main.add_module('Progress')

basf2.process(main)
print(basf2.statistics)
