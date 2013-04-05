#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This steering file creates the Belle II detector geometry, and perfoms the
# simulation with EVTGen as generator and MC based track finding and fitting.
#
# EvtMetaGen and EvtMetaInfo generates and shows event meta data (see example
# in the framework package). Gearbox and Geometry are used to create the Belle2
# detector geometry. The generator used in this example is geant4 particle gun
# (see example in the simulation or generator package). FullSim performs the
# full simulation.
#
# CDCDigitizer creates the detector response in the CDC for the simulated Hits.
# For the PXD and SVD currently the TrueHits are used (created directly by the
# sensitive detector), will be replaced by realistic clusters later on.
#
# MCTrackFinder creates relations between MCParticles and CDCHits/PXDTrueHits/
# SVDTrueHits produced by it. GenFitter fits the found MCTracks and created two
# track collections: GFTracks (Genfit class) and Tracks (class with helix
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
# BelleII.xml, comment the next line
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])
# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
# DECFile and pdlFile have sane defaults
#
# simulation
g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # this is need for the MCTrackFinder to work correctly
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# find MCTracks
mctrackfinder = register_module('MCTrackFinder')

# select which detectors you would like to use
param_mctrackfinder = {'UseCDCHits': 1, 'UseSVDHits': 1, 'UsePXDHits': 1}
mctrackfinder.param(param_mctrackfinder)
# select which particles to use, here as example: only particles which created
# hits in all tracking detectors (PXD, SVD and CDC) but do not have to be
# marked as primary
mctrackfinder.param('WhichParticles', ['PXD', 'SVD', 'CDC'])

# fitting
trackfitting = register_module('GenFitter')

# fit the tracks with one iteration of Kalman filter
# 'PDGCodes': [] means use the pdgCodes from the MCTrackFinder wich are the correct ones
param_trackfitting = {
    'StoreFailedTracks': 0,
    'FilterId': 0,
    'PDGCodes': [],
    'NIterations': 1,
    'ProbCut': 0.001,
    }
trackfitting.param(param_trackfitting)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'MCFittingEvtGenOutput.root')

# create paths
main = create_path()

# add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(trackfitting)
main.add_module(output)

# Process events
process(main)
print statistics
