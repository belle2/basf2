#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Example steering file - 2016 Belle II Collaboration
#
# Modifications for release-00-08: Torben Ferber (ferber@physics.ubc.ca)
#
########################################################


import os
from basf2 import *
from simulation import add_simulation
from reconstruction import add_tracking_reconstruction, add_ecl_modules

set_log_level(LogLevel.ERROR)

# Register necessary modules
eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

# Create geometry
# Geometry parameter loader
gearbox = register_module('Gearbox')

# Geometry builder
geometry = register_module('Geometry')

# Simulation
g4sim = register_module('FullSim')

# one event
eventinfosetter.param({'evtNumList': [3], 'runList': [1]})

import random
intseed = random.randint(1, 10000000)

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22, 111],
    'nTracks': 6,
    'momentumGeneration': 'uniform',
    'momentumParams': [1., 2.],
    'thetaGeneration': 'uniform',
    'thetaParams': [50., 130.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}

pGun.param(param_pGun)

eclDigi = register_module('ECLDigitizer')
eclHit = register_module('ECLHitDebug')

makeMatch = register_module('MCMatcherECLClusters')

# Create paths
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
add_simulation(main)

add_tracking_reconstruction(main)
main.add_module(eclDigi)
main.add_module(eclHit)

add_ecl_modules(main)

simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', 'Output.root')
main.add_module(simpleoutput)

process(main)
print(statistics)
