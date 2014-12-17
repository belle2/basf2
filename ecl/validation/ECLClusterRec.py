#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
# This steering file is an example on how to use
# EclDataAnalysis module.
#
# 1000 500 MeV-electrons are generated with ParticleGun
# and reconstructed. ECL-related infos are dumped on
# a TTree saved in an output file named
# ECLClusterOutput_500mev.root .
#
# Example steering file - 2014 Belle II Collaboration
########################################################

import os
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

# Create paths
main = create_path()

# Event setting and info
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

# random number for generation
import random
intseed = random.randint(1, 10000000)

# single particle generator settings
pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [11],
    'nTracks': 0,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.5],
    'thetaGeneration': 'uniform',
    'thetaParams': [40., 120.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }

pGun.param(param_pGun)
main.add_module(pGun)

add_simulation(main)
add_reconstruction(main)

# eclDataAnalysis module
ecldataanalysis = register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', 'ECLClusterOutput_500mev.root')
ecldataanalysis.param('doTracking', 1)
main.add_module(ecldataanalysis)

process(main)
print statistics
