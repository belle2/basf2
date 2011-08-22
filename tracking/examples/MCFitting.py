#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
# This steering file creates the Belle II detector geometry,
# perfoms the simulation and MC based track finding and fitting.
#
# EvtMetaGen and EvtMetaInfo generates and shows event meta data (see example in the framework package).
# ParamLoaderXML and GeoBuilder are used to create the Belle2 detector geometry.
# The generator used in this example is geant4 particle gun (PGunInput) (see example in the simulation or generator package how to set a random seed).
# FullSim performs the full simulation and PrintMCParticles shows MCParticles information.

# CDCDigi creates the detecotor response in the CDC for the simulated Hits.
# SVDDigi creates the detecotor response in the SVD for the simulated Hits.
# PXDDigi creates the detecotor response in the PXD for the simulated Hits.

# MCTrackFinder creates relations between MCParticles and CDCRecoHits produced by it.
# GenFitter fits the found MCTracks and created two track collections: GFTracks (Genfit class) and Tracks (class with helix parametrization)
#
# For details about module parameters just type > basf2 -m .
#
############################################################################################################################

import os
from basf2 import *

# Register necessary modules
evtmetagen = register_module('EvtMetaGen')
evtmetainfo = register_module('EvtMetaInfo')

# Create geometry
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')

# Simulation
pGun = register_module('PGunInput')
g4sim = register_module('FullSim')
mcparticle = register_module('PrintMCParticles')

# Digitizer
cdcDigitizer = register_module('CDCDigi')
pxdDigitizer = register_module('PXDDigi')
pxdClusterizer = register_module('PXDClusterizer')
svdDigitizer = register_module('SVDDigi')

# Find MCTracks
mctrackfinder = register_module('MCTrackFinder')
# mctrackfinder.logging.log_level = LogLevel.DEBUG

# Fitting
cdcfitting = register_module('GenFitter')
# cdcfitting.logging.log_level = LogLevel.DEBUG

# Output
output = register_module('SimpleOutput')

import random
intseed = random.randint(1, 10000000)

# Set parameters

# one event
evtmetagen.param('EvtNumList', [1])

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

# 5 particles with uniform momentum distribution between 0.5 an 2 GeV
param_pGun = {
    'PIDcodes': [211, -211],
    'Rseed': intseed,
    'nTracks': 5,
    'pPar1': 0.5,
    'pPar2': 2.0,
    'thetaPar1': 87,
    'thetaPar2': 120,
    }

pGun.param(param_pGun)
mcparticle.param('onlyPrimaries', 1)

# Parameters of the MCTrackFinder.
# Input collections: names for MCParticle collection as well as for the collections of Hits.
# Options: select hits from which detectors should be used (all hits selected here are then used in the fit)
# Smearing: smear the true MCMomentum bevor set it as start values for the GFTrackCand (it has to be investigated if it is better for the fit)
# Output collections: names for GFTrackCandidates, relations between GFTrackCandidates and Hits and GFTrackCandidates and MCParticles.
param_mctrackfinder = {
    'MCParticlesColName': 'MCParticles',
    'CDCHitsColName': 'CDCHits',
    'SVDHitsColName': 'SVDHits',
    'PXDHitsColName': 'PXDHits',
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'GFTrackCandidatesColName': 'GFTrackCandidates',
    'GFTrackCandToMCParticleColName': 'GFTrackCandidateToMCParticle',
    'Smearing': 0,
    }
mctrackfinder.param(param_mctrackfinder)

# Input collections: names for Hits and GFTrackCandidates
# Output collections: names for GFTracks and Tracks
# MCTrack and Tracks from pattern recognition have to be handled slightly different (unknown pdg for pattern reco tracks), so set 'mcTracks' to 1 if you are fitting Tracks from MCTrackFinderModule.
param_cdcfitting = {
    'GFTrackCandidatesColName': 'GFTrackCandidates',
    'CDCHitsColName': 'CDCHits',
    'SVDHitsColName': 'SVDHits',
    'PXDHitsColName': 'PXDHits',
    'TracksColName': 'Tracks',
    'GFTracksColName': 'GFTracks',
    'mcTracks': 1,
    }

cdcfitting.param(param_cdcfitting)

# Name of the output root file
output.param('outputFileName', 'FittedMCTracks.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(pGun)
main.add_module(g4sim)
# main.add_module(mcparticle)

main.add_module(cdcDigitizer)
main.add_module(svdDigitizer)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)

main.add_module(mctrackfinder)
main.add_module(cdcfitting)
main.add_module(output)

# Process events
process(main)
