#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""This steering file shows how to use 'ECLHitDebug'
   module to dump ECL-related quantities in an ntuple.

Input:
    No file is required

Output:
    Root file named 'Output.root'

Usage:
    $ basf2 EclHitDebug.py
"""

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_tracking_reconstruction
from reconstruction import add_ecl_modules

__authors__ = ['Poyuan Chen', 'Benjamin Oberhof',
               'Torben Ferber']
__copyright__ = 'Copyright 2016 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

b2.set_log_level(b2.LogLevel.ERROR)

# Register and add 'EventInfoSetter' module and settings
eventInfoSetter = b2.register_module('EventInfoSetter')
eventInfoSetter.param({'evtNumList': [3],
                       'runList': [1],
                       'expList': [0]})  # one event
mainPath.add_module(eventInfoSetter)

# Register and add 'EventInfoPrinter' module
eventInfoPrinter = b2.register_module('EventInfoPrinter')
mainPath.add_module(eventInfoPrinter)

# Create geometry
# Geometry parameter loader
gearbox = b2.register_module('Gearbox')

# Register 'Geometry' module
geometry = b2.register_module('Geometry')

# Register 'FullSim' module
g4sim = b2.register_module('FullSim')

# Random number for generation
b2.set_random_seed(123456)

# Register and add 'ParticleGun' generator module and settings
particleGun = b2.register_module('ParticleGun')
param_particleGun = {
    'pdgCodes': [22, 111],  # 22: photon, 111: pi0
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
particleGun.param(param_particleGun)
mainPath.add_module(particleGun)

# Register and add 'ECLDigitizer' module
eclDigitizer = b2.register_module('ECLDigitizer')
mainPath.add_module(eclDigitizer)

# Register and add 'ECLHitDebug' module
eclHitDebug = b2.register_module('ECLHitDebug')
mainPath.add_module(eclHitDebug)

# Register 'MCMatcherECLClusters' module
mcMatcherECLClusters = b2.register_module('MCMatcherECLClusters')

# Simulation
add_simulation(mainPath)

# Tracking reconstruction
add_tracking_reconstruction(mainPath)

# Add the ECL reconstruction modules to the path
add_ecl_modules(mainPath)

# Register and add 'RootOutput' module
outputFile = b2.register_module('RootOutput')
outputFile.param('outputFileName', 'Output.root')
mainPath.add_module(outputFile)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
