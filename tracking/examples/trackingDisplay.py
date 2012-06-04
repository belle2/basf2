#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

# Create main path
main = create_path()

evtmetagen = register_module('EvtMetaGen')

evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [5])

evtmetainfo = register_module('EvtMetaInfo')

gearbox = register_module('Gearbox')
geo = register_module('Geometry')
# Outer detectors are disabled for performance reasons.
# Note that this may produce a larger number of particles reentering
# the detector from the outside.
geo.param('ExcludedComponents', ['TOP', 'ECL', 'BKLM', 'EKLM'])

# particle gun to shoot particles in the detector
pGun = register_module('ParticleGun')

# choose the particles you want to simulate
param_pGun = {
    'pdgCodes': [211, -211],
    'nTracks': 4,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.4, 1.6],
    'thetaGeneration': 'uniform',
    'thetaParams': [60., 120.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }

pGun.param(param_pGun)

# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigi')

# use one gaussian with resolution of 0.01 in the digitizer
# (to simplify the fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# find MCTracks
mctrackfinder = register_module('MCTrackFinder')

# select which detectors you would like to use
param_mctrackfinder = {'UseCDCHits': 1, 'UseSVDHits': 1, 'UsePXDHits': 1}
# select which particles to use: primary particles
param_mctrackfinder = {'WhichParticles': 0}
mctrackfinder.param(param_mctrackfinder)

# fitting
genfitter = register_module('GenFitter')

# fit the tracks with one iteration of Kalman filter
param_cdcfitting = {
    'StoreFailedTracks': 0,
    'mcTracks': 1,
    'FilterId': 0,
    'NIterations': 1,
    'ProbCut': 0.001,
    }
genfitter.param(param_cdcfitting)

# Add modules to main path
main.add_module(evtmetagen)
main.add_module(gearbox)
main.add_module(geo)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(genfitter)

display = register_module('TrackingDisplay')
# The Options parameter is a combination of:
# A autoscale errors - use when hits are too small to be seen
#   (because of tiny errors)
# D draw detectors - draw simple detector representation (with different size)
#   for each hit
# H draw track hits
# R draw raw simhits (not associated with a track) - drawn as points only
# G draw geometry (whole detector)
# M draw track markers - intersections of track with detector planes
#   (use with T)
# P draw detector planes
# S scale manually - spacepoint hits are drawn as spheres and scaled with
#   errors
# T draw track (straight line between detector planes)
# X silent - open TEve, but don't actually draw anything
#
# Note that you can always turn off an individual detector component or track
# interactively by removing its checkmark in the 'Eve' tab.
display.param('Options', 'MHTGR')  # default

# show all SimHits (default)
display.param('AddPXDHits', True)
display.param('AddSVDHits', True)
display.param('AddCDCHits', True)

main.add_module(display)

process(main)
print statistics
