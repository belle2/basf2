#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
# This steering file executes the full simulation chain to study B0 -> phi Kshort decays:
# o EvtGen is used to generate the following physics events:
#    - Y(4S) -> B0 B0bar (with BBbar mixing turned on)
#    - one neutral B decays always genericaly (b -> c transition)
#    - second neutral B decays always as
#       B0/B0bar -> phi Kshort
#                   phi -> K+ K-
#                   Kshort -> pi+ pi-
# o the user defined decay.dec file is given in analysis/modules/B2PhiKs/datafiles/B0toPhiKshort.dec
#
# In the current version only the tracking and PID sub-detectors are included (no ECL or KLM)
#
# In this version CDC + matched SVD hits are used in track fiting
#
############################################################################################################################

import os

from basf2 import *
set_log_level(LogLevel.INFO)

# ---------------------------------------------------------------
# EvtGen
# boost all momenta to LAB system
# use specified user decay file
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/analysis/modules/B2PhiKs/datafiles/B0toPhiKshort.dec')

# specify number of events to be generated in job
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [10])  # we want to process 10 events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')

# ---------------------------------------------------------------
# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate only tracking+PID detectors
geometry.param('Components', [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    ])

# ---------------------------------------------------------------
# simulation
g4sim = register_module('FullSim')
g4sim.logging.log_level = LogLevel.ERROR
param_g4sim = {'RegisterOptics': 1, 'PhotonFraction': 0.3,
               'TrackingVerbosity': 0}
g4sim.param(param_g4sim)

# ---------------------------------------------------------------
# CDC digitizer
cdcDigitizer = register_module('CDCDigitizer')

# use one gaussian with resolution of 0.01 in the digitizer (to simplify the fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# ---------------------------------------------------------------
# Add CDC background hits
# It doesn't work yet, so it's set to 0
cdcBackground = register_module('CDCSimpleBackground')
cdcBackground.param('BGLevelHits', 0.0)
cdcBackground.param('BGLevelClusters', 0.0)

# ---------------------------------------------------------------
# Track finding is performed with Trasan module
# found tracks are saved to "GFTrackCands_Trasan" collection
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
# Track fitting is performed with GenFit
cdcfitting = register_module('GenFitter')

# set correct collection name as input and custom collection names as output
# select DAF instead of Kalman as Filter
# set the pdg hypothesis to the simulated one, if you want to fit with different pdg hypothesises, set 'allPDG' to true
param_cdcfitting = {
    'GFTrackCandidatesColName': 'GFTrackCands_Trasan',
    'TracksColName': 'Tracks_Trasan',
    'GFTracksColName': 'GFTracks_Trasan',
    'GFTracksToMCParticlesColName': 'GFTracksToMCParticles_Trasan',
    'mcTracks': 0,
    'pdg': 211,
    'allPDG': 0,
    'FilterId': 1,
    'NIterations': 1,
    'ProbCut': 0.001,
    }

cdcfitting.param(param_cdcfitting)

# ---------------------------------------------------------------
# extrapolate to SVD
extrapolate = register_module('ExtrapolateToSVD')

# set the correct input (GFTracks from Trasa) and output (new GFTrackCands with SVDHits) collection names
extrapolate.param('GFTracksColName', 'GFTracks_Trasan')
extrapolate.param('GFTrackCandsColName', 'GFTrackCands_withSVD')

# ---------------------------------------------------------------
# refitting: CDC+matched SVD hits
svdfitting = register_module('GenFitter')

# set proper new collection names (important to avoid mix up with previous collections)
param_svdfitting = {
    'GFTrackCandidatesColName': 'GFTrackCands_withSVD',
    'TracksColName': 'Tracks_withSVD',
    'GFTracksColName': 'GFTracks_withSVD',
    'GFTracksToMCParticlesColName': 'GFTracksToMCParticles_withSVD',
    'StoreFailedTracks': 1,
    'mcTracks': 0,
    'pdg': 211,
    'allPDG': 0,
    'FilterId': 1,
    'ProbCut': 0.001,
    }

svdfitting.param(param_svdfitting)

# ---------------------------------------------------------------
# TOP - digitization
topdigi = register_module('TOPDigitizer')
param_digi = {'PhotonFraction': 0.3}
topdigi.param(param_digi)

# ---------------------------------------------------------------
# TOP reconstruction
topreco = register_module('TOPReconstructor')

# ---------------------------------------------------------------
# ARICH - digitization
arichDigi = register_module('ARICHDigitizer')

# ---------------------------------------------------------------
# ARICH reconstruction
arichRec = register_module('ARICHReconstructor')

# ---------------------------------------------------------------
# Analysis module to study B0 -> Phi Kshort decays
analysis = register_module('B2PhiKs')

# output root file name (the suffix .root will be added automaticaly)
analysis.param('outputFileName', 'BtoPhiKshortWithSVD')

# specify the names of Track collections
# use tracks after refitting (including SVD hits)
analysis.param('GFTrackCandidatesColName', 'GFTrackCands_withSVD')
analysis.param('TracksColName', 'Tracks_withSVD')
analysis.param('GFTracksColName', 'GFTracks_withSVD')

# ---------------------------------------------------------------
# Add all modules to the main path
main = create_path()
main.add_module(evtmetagen)
main.add_module(evtgeninput)

main.add_module(progress)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(g4sim)

main.add_module(cdcDigitizer)
main.add_module(cdcBackground)

main.add_module(trasan)
# main.add_module(cdctracking)

main.add_module(mcmatching)

main.add_module(cdcfitting)

main.add_module(extrapolate)
main.add_module(svdfitting)

main.add_module(topdigi)
main.add_module(topreco)

main.add_module(arichDigi)
main.add_module(arichRec)

main.add_module(analysis)

process(main)

# Print call statistics
print statistics
