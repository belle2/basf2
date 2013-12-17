#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from basf2 import *
from subprocess import call
set_log_level(LogLevel.ERROR)
set_random_seed(3)
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [1000])
eventinfoprinter = register_module('EventInfoPrinter')
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
g4sim.param('StoreAllSecondaries', True)  # this is need for the TrackFinderMCTruth to work correctly
SVDDIGI = register_module('SVDDigitizer')
# SVDDIGI.logging.log_level = LogLevel.DEBUG
SVDCLUST = register_module('SVDClusterizer')
# SVDCLUST.logging.log_level = LogLevel.DEBUG

PXDDIGI = register_module('PXDDigitizer')
# PXDDIGI.logging.log_level = LogLevel.DEBUG
PXDCLUST = register_module('PXDClusterizer')
# PXDCLUST.param("AssumeSorted", False)
track_finder_mc_truth = register_module('TrackFinderMCTruth')
track_finder_mc_truth.logging.log_level = LogLevel.WARNING
param_track_finder_mc_truth = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    }
track_finder_mc_truth.param(param_track_finder_mc_truth)

# mctrackfinder.logging.log_level = LogLevel.DEBUG

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.WARNING
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
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
main.add_module(SVDDIGI)
main.add_module(SVDCLUST)
main.add_module(track_finder_mc_truth)
main.add_module(trackfitter)
main.add_module(trackfitchecker)
# Process events
process(main)
print statistics
