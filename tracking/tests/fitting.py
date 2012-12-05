#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
# This steering files tests if track fitting with the full detector geometry works without a crash
# Track finding is not tested. Only MCTrackFinder is used.
############################################################################################################################

import os
from basf2 import *
set_log_level(LogLevel.ERROR)
set_random_seed(3)
# register necessary modules
evtmetagen = register_module('EvtMetaGen')

# generate one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [10])

evtmetainfo = register_module('EvtMetaInfo')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# particle gun to shoot particles in the detector
pGun = register_module('ParticleGun')

# choose the particles you want to simulate
param_pGun = {
    'pdgCodes': [
        13,
        -13,
        -211,
        211,
        -11,
        11,
        ],
    'nTracks': 4,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.1, 5],
    'thetaGeneration': 'uniform',
    'thetaParams': [25., 145.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [-0.5, 0.5],
    'yVertexParams': [-0.5, 0.5],
    'zVertexParams': [-0.5, 0.5],
    }

pGun.param(param_pGun)

# simulation
g4sim = register_module('FullSim')

# cdc digitizer
cdcDigitizer = register_module('CDCDigitizer')

# use one gaussian with resolution of 0.01 in the digitizer (to simplify the fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# find MCTracks
mctrackfinder = register_module('MCTrackFinder')
param_mctrackfinder = {'UseCDCHits': 1, 'UseSVDHits': 1, 'UsePXDHits': 1}
mctrackfinder.param(param_mctrackfinder)
mctrackfinder.logging.log_level = LogLevel.INFO
# fitting
trackFitter = register_module('GenFitter')

# fit the tracks with one iteration of Kalman filter
param_fitting = {'StoreFailedTracks': 0, 'FilterId': 0, 'NIterations': 1}
trackFitter.param(param_fitting)
trackFitter.logging.log_level = LogLevel.WARNING
trackfitchecker = register_module('TrackFitChecker')
trackfitchecker.logging.log_level = LogLevel.INFO  # the reults of the statistical tests will only show up at info or debug level
trackfitchecker.param('robustTests', True)
# output

# create paths
main = create_path()

# add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)

main.add_module(cdcDigitizer)

main.add_module(mctrackfinder)
main.add_module(trackFitter)
main.add_module(trackfitchecker)

# Process events
process(main)

print statistics
