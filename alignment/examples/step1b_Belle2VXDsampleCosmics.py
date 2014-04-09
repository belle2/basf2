#!/usr/bin/env python
# -*- coding: utf-8 -*-

# BELLE II VXD ALIGNMENT
# STEP 1a:
# Generation of sample of cosmic tracks

# It contains:
# - geometry loading (incl. B field if requested)
# - full sim + cosmics generator
# - digi/clust of PXD/SVD
# - track finding using TrackFinderMCTruth
# - fitting with genfit2's Kalman filter
# - progress
# - output of ROOT objects

import os
import sys

from basf2 import *
set_log_level(LogLevel.WARNING)

# Important parameters of the simulation:
events = 100000  # Number of events to simulate

# ----------------------------
# Event MetaData
# ----------------------------

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})

# ----------------------------
# Geometry
# ----------------------------

# Load Geometry module
gearbox = register_module('Gearbox')

geometry = register_module('Geometry')

# Magnetic field turned off
geometry.param('components', ['BeamPipe', 'PXD', 'SVD'])

# Cosmics generator
# to run the framework the used modules need to be registered
cosmics = register_module('Cosmics')
cosmics.param('level', 1)
cosmics.param('ipRequirement', 1)
# ipdr and ipdz are only relevant for level = 1 and ipRequirement = 1
cosmics.param('ipdr', 3.)
cosmics.param('ipdz', 3.)
cosmics.param('ptmin', 0.7)

# --------------------------------------
# Particle generation and full simulation
# --------------------------------------

# Full simulation module
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', True)

# ----------------------------
# Digitization
# ----------------------------

# SVD digi module
SVDDigi = register_module('SVDDigitizer')
# PXD digi module
PXDDigi = register_module('PXDDigitizer')

# ----------------------------
# Clusterization
# ----------------------------

# SVD clusterizer module
SVDClust = register_module('SVDClusterizer')
# PXD clusterizer module
PXDClust = register_module('PXDClusterizer')

# ----------------------------
# Track finding
# ----------------------------

# Use truth information to create track candidates
mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.logging.log_level = LogLevel.WARNING
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'UseClusters': True,
    'GFTrackCandidatesColName': 'caTracks',
    'WhichParticles': ['SVD'],
    }
mctrackfinder.param(param_mctrackfinder)

# ----------------------------
# Kalman fitting
# ----------------------------
genfit = register_module('GenFitter')
genfit.param('GFTrackCandidatesColName', 'caTracks')

# ---------------------------------------
# Data Output of collections to ROOT file
# ---------------------------------------

# Add output module
output = register_module('RootOutput')
output.param('outputFileName', 'sampleCosmics.root')

# ------------------------------------------
# Some info / helper modules / Event Display
# ------------------------------------------

# Add Progress
progress = register_module('Progress')

# Add event display module
display = register_module('Display')
# Use this parameter to display TB geometry (not Belle2 geometry extract)
display.param('fullGeometry', True)
display.param('GFTrackCandidatesColName', 'caTracks')
display.param('options', 'DHMPS')

# ----------------------------
# Path construction
# ----------------------------

# create the main path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(cosmics)
main.add_module(simulation)
main.add_module(PXDDigi)
main.add_module(SVDDigi)
main.add_module(PXDClust)
main.add_module(SVDClust)
main.add_module(mctrackfinder)
main.add_module(genfit)
main.add_module(output)
# main.add_module(display)
main.add_module(progress)

# Run
process(main)
print statistics
