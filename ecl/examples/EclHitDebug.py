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
import random
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_tracking_reconstruction
from reconstruction import add_ecl_modules

b2.set_log_level(b2.LogLevel.ERROR)

# Register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfoprinter = b2.register_module('EventInfoPrinter')

# Create geometry
# Geometry parameter loader
gearbox = b2.register_module('Gearbox')

# Geometry builder
geometry = b2.register_module('Geometry')

# Simulation
g4sim = b2.register_module('FullSim')

# one event
eventinfosetter.param({'evtNumList': [3],
                       'runList': [1],
                       'expList': [0]})

intseed = random.randint(1, 10000000)

pGun = b2.register_module('ParticleGun')
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

eclDigi = b2.register_module('ECLDigitizer')
eclHit = b2.register_module('ECLHitDebug')

makeMatch = b2.register_module('MCMatcherECLClusters')

# Create paths
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
add_simulation(main)

add_tracking_reconstruction(main)
main.add_module(eclDigi)
main.add_module(eclHit)

add_ecl_modules(main)

simpleoutput = b2.register_module('RootOutput')
simpleoutput.param('outputFileName', 'Output.root')
main.add_module(simpleoutput)

b2.process(main)
print(b2.statistics)
