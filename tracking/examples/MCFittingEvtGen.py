#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This steering file creates the Belle II detector geometry, and perfoms the
# simulation with EVTGen as generator and MC based track finding and fitting.
#
# EventInfoSetter and EventInfoPrinter generates and shows event meta data (see example
# in the framework package). Gearbox and Geometry are used to create the Belle2
# detector geometry. The generator used in this example is geant4 particle gun
# (see example in the simulation or generator package). FullSim performs the
# full simulation.
#
# CDCDigitizer creates the detector response in the CDC for the simulated Hits.
# For the PXD and SVD currently the TrueHits are used (created directly by the
# sensitive detector), will be replaced by realistic clusters later on.
#
# TrackFinderMCTruth creates relations between MCParticles and CDCHits/PXDTrueHits/
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
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [1])
eventinfoprinter = register_module('EventInfoPrinter')

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
# this is need for the TrackFinderMCTruth to work correctly
g4sim.param('StoreAllSecondaries', True)
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# find MCTracks
track_finder_mc_truth = register_module('TrackFinderMCTruth')

# select which detectors you would like to use
param_track_finder_mc_truth = {
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'UseClusters': False,
    }
track_finder_mc_truth.param(param_track_finder_mc_truth)
# select which particles to use, here as example: only particles which created
# hits in all tracking detectors (PXD, SVD and CDC) but do not have to be
# marked as primary
track_finder_mc_truth.param('WhichParticles', ['PXD', 'SVD', 'CDC'])

# fitting
trackfitting = register_module('GenFitter')

# fit the tracks with one iteration of Kalman filter
# 'PDGCodes': [] means use the pdgCodes from
# the TrackFinderMCTruth wich are the correct ones
param_trackfitting = {
    'StoreFailedTracks': 0,
    'FilterId': 0,
    'PDGCodes': [],
    'NIterations': 1,
    'ProbCut': 0.001,
    'UseClusters': False,
    }
trackfitting.param(param_trackfitting)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'MCFittingEvtGenOutput.root')

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(track_finder_mc_truth)
main.add_module(trackfitting)
main.add_module(output)

# Process events
process(main)
print statistics
