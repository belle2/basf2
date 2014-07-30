#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

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
eventinfosetter.param('evtNumList', [100])

import random
intseed = random.randint(1, 10000000)

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

cdcDigitizer = register_module('CDCDigitizer')

ext = register_module('Ext')
ext.param('GFTracksColName', 'GFTracks')
ext.param('ExtTrackCandsColName', 'ExtTrackCands')
ext.param('ExtRecoHitsColName', 'ExtRecoHits')

genfit = register_module('GenFitter')
mctrackfinder = register_module('MCTrackFinder')
param_mctrackfinder = {
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    }
mctrackfinder.param(param_mctrackfinder)

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('NIterations', 3)

trackfitchecker = register_module('TrackFitChecker')
trackfitchecker.logging.log_level = LogLevel.INFO
trackfitchecker.param('testSi', True)
trackfitchecker.param('testCdc', False)
trackfitchecker.param('writeToTextFile', True)

# EKLM Modules
eklmDigitizer = register_module('EKLMDigitizer')
eklmReconstructor = register_module('EKLMReconstructor')
eklmK0LReconstructor = register_module('EKLMK0LReconstructor')

output = register_module('RootOutput')
output.param('outputFileName', 'EKLMFullSim.root')

# Create paths
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
# main.add_module(trackfitchecker)
main.add_module(ext)
main.add_module(eklmDigitizer)
main.add_module(eklmReconstructor)
main.add_module(eklmK0LReconstructor)
main.add_module(output)

process(main)
print statistics
