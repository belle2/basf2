#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from basf2 import *
set_log_level(LogLevel.ERROR)
# register the modules and set there options
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [100])

evtmetainfo = register_module('EvtMetaInfo')

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'PXD', 'SVD', 'CDC'])  # only the tracking detectors will be simulated. Makes this example much faster

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [-13, 13],
    'nTracks': 10,
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

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# mctrackfinder
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
trackfitter.param('NIterations', 2)

trackfitchecker = register_module('TrackFitChecker')
trackfitchecker.logging.log_level = LogLevel.INFO  # the reults of the statistical tests will only show up at info or debug level
# trackfitchecker.param('testSi', True)
trackfitchecker.param('robustTests', True)
trackfitchecker.param('testCdc', False)
trackfitchecker.param('writeToTextFile', True)  # especially when CDC hits are anlyzed the the info on the termianl becomes unreadable. look into the file statisticaltests.txt instead (at the moment the file name is hard coded

# Create paths
main = create_path()
# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
main.add_module(trackfitchecker)
# Process events
process(main)
