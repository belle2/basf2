#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering shows the use of the vertexer module. The Vertexer modules uses
# Rave and Genfit for Vertex finding and fitting
import os
from basf2 import *
set_log_level(LogLevel.ERROR)
set_random_seed(3)

# register the modules and set there options
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [100])
evtmetainfo = register_module('EvtMetaInfo')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
vertexStd = 1  # 1 mm
vertexVar = vertexStd ** 2
pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [-13, 13],
    'nTracks': 3,
    'momentumGeneration': 'uniform',
    'momentumParams': [1, 1],
    'thetaGeneration': 'uniform',
    'thetaParams': [60., 120.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0],
    'yVertexParams': [0.0, 0],
    'zVertexParams': [0.0, 0],
    }
    # 'vertexGeneration': 'normal', 'xVertexParams': [0.0, vertexStd],
    # 'yVertexParams': [0.0, vertexStd], 'zVertexParams': [0.0, vertexStd],
pGun.param(param_pGun)
g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # this is need for the MCTrackFinder to work correctly

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# find MC tracks
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
# the reults of the statistical tests will only show up at info or debug level
trackfitchecker.logging.log_level = LogLevel.INFO
trackfitchecker.param('robustTests', True)
trackfitchecker.param('writeToTextFile', True)
trackfitchecker.param('writeToRootFile', True)
vertexer = register_module('Vertexer')
vertexer.logging.log_level = LogLevel.WARNING

# here you can choose the propagation method used during vertexing. Using the
# Genfit propagation is default If this parameter is set the false the Rave
# interal propagation is used. The Rave internal propagation ignores material.
vertexer.param('useGenfitPropagation', True)

# here you see how to give the beam spot info to the vertexer module. This does
# not seem to work correctly at the moment vertexer.param('useBeamSpot', True)
# vertexer.param('beamSpotPosition', [0.0,0.0,0.0]) beamSpotCov =
# [vertexVar,0.0,0.0, 0.0,vertexVar,0.0, 0.0,0.0,vertexVar]
# vertexer.param('beamSpotCovariance', beamSpotCov)
#
# here are the different algorithms Rave supports for vertex reconstuciton
vertexer.param('vertexingMethod', 'kalman-smoothing:1')  # Kalman filter

# adaptive vertex fitter -- a Kalman filter based robust vertex fitter (similar
# to the DAF for track fitting) vertexer.param('vertexingMethod','avf-
# Tini:256-ratio:0.25-sigmacut:3-smoothing:1')
#
# This methed has still problems! vertexer.param('vertexingMethod','tkf-
# ptcut:0-trkcutpv:0.05-trkcutsv:0.01-vtxcut:0.01-smoothing:1')
#
# the VertexFitChecker will calculated the vertex resolution, pulls and p
# values
vertexfitchecker = register_module('VertexFitChecker')
vertexfitchecker.logging.log_level = LogLevel.INFO
vertexfitchecker.param('writeToTextFile', True)
vertexfitchecker.param('writeToRootFile', True)
vertexfitchecker.param('robustTests', True)

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
main.add_module(vertexer)
main.add_module(vertexfitchecker)
# Process events
process(main)
print statistics
