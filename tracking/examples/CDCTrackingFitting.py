#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
# This steering file creates the Belle II detector geometry,
# perfoms the simulation and pattern recognition, MC based track finding and fitting in the CDC.
#
# EvtMetaGen and EvtMetaInfo generates and shows event meta data (see example in the framework package).
# ParamLoaderXML and GeoBuilder are used to create the Belle2 detector geometry.
# The generator used in this example is geant4 particle gun (PGunInput) (see example in the simulation or generator package how to set a random seed).
# FullSim performs the full simulation and PrintMCParticles shows MCParticles information.
# CDCDigi creates Hits in the CDC from simulated Hits.
# CDCRecoHitMaker creates RecoHits, which are needed and used during the fit.
# CDCTracking performs pattern recognition in the CDC (the digitized Hits are collected to create CDCTrackCandidates).
# CDCMCMatching matches to each CDCTrackCandidate the corresponding MCParticle.
# MCTrackFinder creates relations between MCParticles and CDCRecoHits produced by it.
# GenFitter fits the found tracks (MCTracks as well as CDCTrackCandidates)
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

# Digitizer in the CDC
cdcdigitizer = register_module('CDCDigi')

# Create CDCRecoHits
recohitmaker = register_module('CDCRecoHitMaker')

# Pattern Recognition
cdctracking = register_module('CDCTracking')

# MCMatching
cdcmcmatching = register_module('CDCMCMatching')

# Find MCTracks
mctrackfinder = register_module('MCTrackFinder')

# Fitting
cdcfitting = register_module('GenFitter')

# Output
output = register_module('SimpleOutput')

import random
intseed = random.randint(1, 10000000)

# Set parameters

# one event
evtmetagen.param('EvtNumList', [1])

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

# 2 particles with uniform momentum distribution between 1 an 2 GeV
param_pGun = {
    'PIDcodes': [211],
    'Rseed': 1111111,
    'nTracks': 2,
    'pPar1': 1.0,
    'pPar2': 2.0,
    'thetaPar1': 67,
    'thetaPar2': 100,
    }

pGun.param(param_pGun)
mcparticle.param('onlyPrimaries', 1)

param_cdctracking = {
    'CDCSimHitsColName': 'CDCSimHits',
    'CDCHitsColName': 'CDCHits',
    'CDCRecoHitsColName': 'CDCRecoHits',
    'CDCTrackCandidatesColName': 'CDCTrackCandidates',
    'CDCTrackCandsToCDCRecoHitsColName': 'CDCTrackCandidatesToCDCRecoHits',
    'TextFileOutput': 0,
    }
cdctracking.param(param_cdctracking)

param_cdcmcmatching = {
    'MCParticlesColName': 'MCParticles',
    'MCParticleToCDCSimHitsColName': 'MCPartToCDCSimHits',
    'CDCSimHitToCDCHitColName': 'SimHitToCDCHits',
    'CDCRecoHitsColName': 'CDCRecoHits',
    'CDCTrackCandidatesColName': 'CDCTrackCandidates',
    'CDCTrackCandsToCDCRecoHitsColName': 'CDCTrackCandidatesToCDCRecoHits',
    'CDCTrackCandsToMCParticlesColName': 'CDCTrackCandidateToMCParticle',
    }
cdcmcmatching.param(param_cdcmcmatching)

param_mctrackfinder = {
    'MCParticlesColName': 'MCParticles',
    'MCParticleToCDCSimHitsColName': 'MCPartToCDCSimHits',
    'CDCSimHitToCDCHitColName': 'SimHitToCDCHits',
    'CDCRecoHitsColName': 'CDCRecoHits',
    'TracksColName': 'Tracks',
    'TrackToCDCRecoHitColName': 'TrackToCDCRecoHits',
    'TrackToMCParticleColName': 'TrackToMCParticle',
    }
mctrackfinder.param(param_mctrackfinder)

param_cdcfitting = {
    'MCParticlesColName': 'MCParticles',
    'CDCRecoHitsColName': 'CDCRecoHits',
    'TracksColName': 'Tracks',
    'TrackToCDCRecoHitColName': 'TrackToCDCRecoHits',
    'TrackToMCParticleColName': 'TrackToMCParticle',
    'CDCTrackCandidatesColName': 'CDCTrackCandidates',
    'CDCTrackCandsToCDCRecoHitsColName': 'CDCTrackCandidatesToCDCRecoHits',
    'CDCTrackCandsToMCParticlesColName': 'CDCTrackCandidateToMCParticle',
    'FitMCTracks': 1,
    'FitRecoTracks': 1,
    }
cdcfitting.param(param_cdcfitting)

output.param('outputFileName', 'CDCTrackFitOutput.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(mcparticle)
main.add_module(cdcdigitizer)
main.add_module(recohitmaker)
main.add_module(cdctracking)
main.add_module(cdcmcmatching)
main.add_module(mctrackfinder)
main.add_module(cdcfitting)
# main.add_module(output)

# Process events
process(main)
