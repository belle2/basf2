#!/usr/bin/env python
# -*- coding: utf-8 -*-

# BELLE II VXD ALIGNMENT
# STEP 1a:
# Generation of sample of muons from IP

# It contains:
# - geometry loading (incl. B field if requested)
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
events = 50000  # Number of events to simulate

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
# CDC is added because of possible later physics validation (Simon Wehle) ... with just VXD alignment so far
geometry.param('components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD'])

# ParticleGun
particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', 2)
# muons/antimouns
particlegun.param('pdgCodes', [13, -13])
particlegun.param('independentVertices', True)

# --------------------------------------
# Partcle generation and full simulation
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
genfit.param('UseClusters', True)
genfit.param('GFTrackCandidatesColName', 'caTracks')

# ---------------------------------------
# Data Output of collections to ROOT file
# ---------------------------------------

# Add output module
output = register_module('RootOutput')
output.param('outputFileName', 'sampleIP.root')

# ------------------------------------------
# Some info / helper modules / Event Display
# ------------------------------------------

# Add Progress
progress = register_module('Progress')

# Add event display module
display = register_module('Display')
# display.param('fullGeometry', True)
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
main.add_module(particlegun)
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
