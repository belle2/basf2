#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
# This steering file creates the Belle II detector geometry,
# and perfoms the simulation and MC based track finding and fitting.
#
# EvtMetaGen and EvtMetaInfo generates and shows event meta data (see example in the framework package).
# Gearbox and Geometry are used to create the Belle2 detector geometry.
# The generator used in this example is geant4 particle gun (see example in the simulation or generator package).
# FullSim performs the full simulation.

# CDCDigi creates the detector response in the CDC for the simulated Hits.
# For the PXD and SVD currently the TrueHits are used (created directly by the sensitive detector), will be replaced by realistic clusters later on.

# MCTrackFinder creates relations between MCParticles and CDCHits/PXDTrueHits/SVDTrueHits produced by it.
# GenFitter fits the found MCTracks and created two track collections: GFTracks (Genfit class) and Tracks (class with helix parametrization)
#
# For details about module parameters just type > basf2 -m .
#
############################################################################################################################

import os
from basf2 import *

# Register necessary modules
evtmetagen = register_module('EvtMetaGen')

# one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [1])

evtmetainfo = register_module('EvtMetaInfo')

# Create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# if you want you can simulate only tracking detectors, if you want to simulate the whole detector, comment the next line out
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])

# Simulation
pGun = register_module('ParticleGun')

# choose the particles you want to simulate
param_pGun = {
    'pdgCodes': [13, -13],
    'randomSeed': 1028307,
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.8, 1.2],
    'thetaGeneration': 'fixed',
    'thetaParams': [100., 100.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }

pGun.param(param_pGun)

g4sim = register_module('FullSim')

# Digitizer
cdcDigitizer = register_module('CDCDigi')

# use one gaussian with resolution of 0.01 in the digitizer
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# Find MCTracks
mctrackfinder = register_module('MCTrackFinder')

# select which detectors you would like to use and if the values storen in the track candidates should be smeared
param_mctrackfinder = {
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    }
mctrackfinder.param(param_mctrackfinder)

# Fitting
cdcfitting = register_module('GenFitter')

# fit the tracks with Kalman filter
param_cdcfitting = {
    'StoreFailedTracks': 0,
    'mcTracks': 1,
    'FilterId': 0,
    'NIterations': 1,
    'ProbCut': 0.001,
    }
cdcfitting.param(param_cdcfitting)

# Output
output = register_module('SimpleOutput')
output.param('outputFileName', 'MCFittingOutput.root')

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
main.add_module(cdcfitting)

main.add_module(output)

# Process events
process(main)

print statistics
