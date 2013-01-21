#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from basf2 import *
set_log_level(LogLevel.ERROR)
set_random_seed(1)
# register the modules and set there options
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [10000])

evtmetainfo = register_module('EvtMetaInfo')

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])  # only the tracking detectors will be simulated. Makes this example much faster

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [1.0, 1.0],
    'thetaGeneration': 'fixed',
    'thetaParams': [110., 110.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }
pGun.param(param_pGun)

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # this is need for the MCTrackFinder to work correctly

mctrackfinder = register_module('MCTrackFinder')
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    }
mctrackfinder.param(param_mctrackfinder)

trackfitter = register_module('GenFitter2')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitchecker = register_module('TrackFitChecker')
trackfitchecker.logging.log_level = LogLevel.INFO  # the reults of the statistical tests will only show up at info or debug level
trackfitchecker.param('testSi', True)
trackfitchecker.param('writeToTextFile', True)
trackfitchecker.param('truthAvailable', True)
trackfitchecker.param('robustTests', True)
trackfitchecker.param('writeToRootFile', True)
trackfitchecker.param('inspectTracks', False)
trackfitchecker.param('outputFileName', 'siTracking')

# Create paths
main = create_path()
# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
main.add_module(trackfitchecker)
# Process events
process(main)
