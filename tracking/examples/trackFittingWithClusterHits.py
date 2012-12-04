#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from basf2 import *
from subprocess import call
set_log_level(LogLevel.ERROR)
set_random_seed(3)
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [1000])
evtmetainfo = register_module('EvtMetaInfo')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# only the tracking detectors will be simulated. Makes this example much faster
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

pGun = register_module('ParticleGun')
# Set parameters for particlegun
pGun.param({
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.7, 0.7],
    'thetaGeneration': 'uniform',
    'thetaParams': [50, 70.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    })
g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # this is need for the MCTrackFinder to work correctly
SVDDIGI = register_module('SVDDigitizer')
# SVDDIGI.logging.log_level = LogLevel.DEBUG
SVDCLUST = register_module('SVDClusterizer')
# SVDCLUST.logging.log_level = LogLevel.DEBUG

PXDDIGI = register_module('PXDDigitizer')
# PXDDIGI.logging.log_level = LogLevel.DEBUG
PXDCLUST = register_module('PXDClusterizer')
# PXDCLUST.param("AssumeSorted", False)
mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.logging.log_level = LogLevel.WARNING
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    }
mctrackfinder.param(param_mctrackfinder)

# mctrackfinder.logging.log_level = LogLevel.DEBUG

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('UseClusters', True)
trackfitchecker = register_module('TrackFitChecker')
trackfitchecker.logging.log_level = LogLevel.INFO  # the reults of the
                                                   # statistical tests will
                                                   # only show up at info or
                                                   # debug level
trackfitchecker.param('testSi', False)
trackfitchecker.param('robustTests', True)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
main.add_module(SVDDIGI)
main.add_module(SVDCLUST)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
main.add_module(trackfitchecker)
# Process events
process(main)
print statistics
