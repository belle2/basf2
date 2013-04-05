#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
############################################## This steering file is an
# example how to execute MC based track finding and fitting and 'realistic'
# track finding and fitting in one file. The results are stored in the
# TrackingOutput branch and allows a comparison between the two ways, and thus
# allows an evaluation of the pattern recognition. This is just an example of
# how the evaluation can be performed, it is surely can and should be improved
# in the future, so it is just meant to be an orientation help for people who
# are starting to work on tracking in basf2 and not as ultimate solution.
#
# This steering file creates the Belle II detector geometry, perfoms the
# simulation, mc based and realistic pattern recognition in der CDC. Afterwards
# the resulting tracks are fitted and stored in a suitable way.
#
# EvtMetaGen and EvtMetaInfo generates and shows event meta data (see example
# in the framework package). Gearbox and Geometry are used to create the Belle2
# detector geometry. The generator used in this example is geant4 particle gun
# (see example in the simulation or generator package). FullSim performs the
# full simulation.
#
# CDCDigitizer creates the detecotor response in the CDC for the simulated
# Hits.
#
# MCTrackFinder creates relations between MCParticles and
# CDCHits/PXDTrueHits/SVDTrueHits produced by it. CDCTracking performs pattern
# recognition in the CDC based on conformal algorithm. GFTrackCandidates with
# corresponding hit indices and start values are created. GenFitter fits the
# found GFTrackCandidates and created two track collections: GFTracks (Genfit
# class) and Tracks (class with helix parametrization)
#
# TrackingOutput creates the TrackingOutput objects to store the results.
#
# For details about module parameters just type > basf2 -m .
#
##############################################################################
###############################################

import os
from basf2 import *

# register necessary modules
evtmetagen = register_module('EvtMetaGen')

# generate one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [1])
evtmetainfo = register_module('EvtMetaInfo')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate only tracking detectors to simulate the whole detector included in
# BelleII.xml, comment the next line out
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])

# shoot particles in the detector
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
# make simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR
g4sim.param('StoreAllSecondaries', True)  # this is need for the MCTrackFinder to work correctly
# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# find MCTracks
mctrackfinder = register_module('MCTrackFinder')

# select which detectors you would like to use and select which particles to use: primary particles
param_mctrackfinder = {
    'UseCDCHits': 1,
    'UseSVDHits': 0,
    'UsePXDHits': 0,
    'WhichParticles': ['primary'],
    }
mctrackfinder.param(param_mctrackfinder)

# fitting of MCTracks
mcfitting = register_module('GenFitter')

# fit the tracks with one iteration of Kalman filter
param_mcfitting = {
    'StoreFailedTracks': 0,
    'FilterId': 0,
    'NIterations': 1,
    'ProbCut': 0.001,
    }
mcfitting.param(param_mcfitting)

# pattern recognition
cdctracking = register_module('CDCTracking')

# give the collection a custom name to mark that it is coming from pattern
# recognition
param_cdctracking = {'GFTrackCandidatesColName': 'GFTrackCands_PatternReco'}
cdctracking.param(param_cdctracking)

# match the found track candidates with MCParticles
mcmatching = register_module('CDCMCMatching')

# select the correct collection for the matching
param_mcmatching = {'GFTrackCandidatesColName': 'GFTrackCands_PatternReco'}
mcmatching.param(param_mcmatching)

# fitting
cdcfitting = register_module('GenFitter')

# set correct collection name as input and custom collection names as output
# select DAF instead of Kalman as Filter set the pdg hypothesis to the
# simulated one, if you want to fit with different pdg hypothesises, set
# 'allPDG' to true
param_cdcfitting = {
    'GFTrackCandidatesColName': 'GFTrackCands_PatternReco',
    'TracksColName': 'Tracks_PatternReco',
    'GFTracksColName': 'GFTracks_PatternReco',
    'PDGCodes': [211],
    'FilterId': 1,
    'NIterations': 1,
    'ProbCut': 0.001,
    }
cdcfitting.param(param_cdcfitting)

# create TrackingOutputObjects
trackingoutput = register_module('TrackingOutput')

# output
output = register_module('RootOutput')
# write out only the interesting branch
output.param('branchNames', ['TrackingOutputs'])
output.param('outputFileName', 'TrackingEvaluationOutput.root')

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
main.add_module(mcfitting)
main.add_module(cdctracking)
main.add_module(mcmatching)
main.add_module(cdcfitting)
main.add_module(trackingoutput)
main.add_module(output)

# Process events
process(main)
print statistics
