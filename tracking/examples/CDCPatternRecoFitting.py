#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This steering file creates the Belle II detector geometry, perfoms the
# simulation and pattern recognition in der CDC. Afterwards the resulting
# tracks are fitted.
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
# CDCTracking performs pattern recognition in the CDC based on conformal
# algorithm. GFTrackCandidates with corresponding hit indices and start values
# are created. GenFitter fits the found GFTrackCandidates and created two track
# collections: GFTracks (Genfit class) and Tracks (class with helix
# parametrization)
#
# For details about module parameters just type > basf2 -m .
#
##############################################################################

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
    'thetaGeneration': 'fixed',
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

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

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
# simulated one, if you want to fit with different pdg hypothesises, just state them all in the PDGCodes list
param_cdcfitting = {
    'GFTrackCandidatesColName': 'GFTrackCands_PatternReco',
    'PDGCodes': [211],
    'FilterId': 1,
    'NIterations': 1,
    'ProbCut': 0.001,
    }
cdcfitting.param(param_cdcfitting)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'CDCPatternRecoOutput.root')

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
main.add_module(cdctracking)
main.add_module(mcmatching)
main.add_module(cdcfitting)
main.add_module(output)

# Process events
process(main)
print statistics
