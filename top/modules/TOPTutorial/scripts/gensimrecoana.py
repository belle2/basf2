#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# This steering file which shows all usage options for the particle gun module
# in the generators package. The generated particles from the particle gun are
# then fed through a full Geant4 simulation and the output is stored in a root
# file.
#
# The different options for the particle gun are explained below.
# Uncomment/comment different lines to get the wanted settings
#
# Example steering file - 2012 Belle II Collaboration
##############################################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# Initialize EvtGen
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', 'B2Kpi.dec')

# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
geometry.param('Components', [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    ])
# Run simulation
simulation = register_module('FullSim')
param_g4sim = {'RegisterOptics': 1, 'PhotonFraction': 0.3,
               'TrackingVerbosity': 0}
simulation.param(param_g4sim)
# Reconstruction
topdigi = register_module('TOPDigitizer')
param_digi = {'PhotonFraction': 0.3}
topdigi.param(param_digi)
topreco = register_module('TOPReconstructor')

# ARICH reconstruction module
arichDigi = register_module('ARICHDigitizer')
arichRec = register_module('ARICHReconstructor')

# ---------------------------------------------------------------
# digitizer
cdcDigitizer = register_module('CDCDigitizer')
# use one gaussian with resolution of 0.01 in the digitizer (to simplify the fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# ---------------------------------------------------------------
# digitizer
cdcBackground = register_module('CDCSimpleBackground')
cdcBackground.param('BGLevelHits', 0.0)
cdcBackground.param('BGLevelClusters', 0.0)

# ---------------------------------------------------------------
# Trasan
trasan = register_module('Trasan')
trasan.param('DebugLevel', 10)
trasan.param('GFTrackCandidatesColName', 'GFTrackCands_Trasan')

# ---------------------------------------------------------------
# match the found track candidates with MCParticles
mcmatching = register_module('CDCMCMatching')

# select the correct collection for the matching
param_mcmatching = {'GFTrackCandidatesColName': 'GFTrackCands_Trasan'}
mcmatching.param(param_mcmatching)

# ---------------------------------------------------------------
# fitting
cdcfitting = register_module('GenFitter')

# set correct collection name as input and custom collection names as output
# select DAF instead of Kalman as Filter
# set the pdg hypothesis to the simulated one, if you want to fit with different pdg hypothesises, set 'allPDG' to true
param_cdcfitting = {
    'GFTrackCandidatesColName': 'GFTrackCands_Trasan',
    'TracksColName': 'Tracks_Trasan',
    'GFTracksColName': 'GFTracks',
    'mcTracks': 1,
    'pdg': 211,
    'allPDG': 0,
    'FilterId': 1,
    'NIterations': 1,
    'ProbCut': 0.001,
    }

cdcfitting.param(param_cdcfitting)

# ---------------------------------------------------------------

toptut = register_module('TOPTutorial')

# Setting the option for all non particle gun modules:
# want to process 100 MC events
evtmetagen.param({'EvtNumList': [10], 'RunList': [1]})

# Set output filename
# output.param('outputFileName', 'TOPOutput.root')

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(simulation)
main.add_module(cdcDigitizer)
main.add_module(cdcBackground)
main.add_module(trasan)
main.add_module(mcmatching)
main.add_module(cdcfitting)

main.add_module(topdigi)
main.add_module(topreco)
main.add_module(arichDigi)
main.add_module(arichRec)

main.add_module(toptut)

# Process events
process(main)

# Print call statistics
print statistics
