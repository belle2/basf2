#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
# This steering file creates the Belle II detector geometry,
# perfoms the simulation and pattern recognition in der CDC. Subsequent one also can fit the resulting tracks.
#
# EvtMetaGen and EvtMetaInfo generates and shows event meta data (see example in the framework package).
# ParamLoaderXML and GeoBuilder are used to create the Belle2 detector geometry.
# The generator used in this example is geant4 particle gun (PGunInput) (see example in the simulation or generator package how to set a random seed).
# FullSim performs the full simulation and PrintMCParticles shows MCParticles information.

# CDCDigi creates the detecotor response in the CDC for the simulated Hits.
# SVDDigi creates the detecotor response in the SVD for the simulated Hits.
# PXDDigi creates the detecotor response in the PXD for the simulated Hits.

# CDCRecoHitMaker creates RecoHits, which are needed and used during the fit.
# SVDRecoHitMaker creates RecoHits, which are needed and used during the fit.
# PXDRecoHitMaker creates RecoHits, which are needed and used during the fit.

# CDCTracking performs pattern recognition in the CDC based on conformal algorithm. GFTrackCandidates and corresponding relations between TrackCandidates and CDCRecoHits are created.
# GenFitter fits the found GFTrackCandidates and created two track collections: GFTracks (Genfit class) and Tracks (class with helix parametrization)
# Caution: it is still under testing how well GenFitter can handle 'unperfect' pattern reco track candidates, so the fitting may fail here...
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

# Create RecoHits
cdcrecohitmaker = register_module('CDCRecoHitMaker')

# Pattern Recognition
cdctracking = register_module('CDCTracking')

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

# Input collections: all types of CDCHits
# Output collections: names of TrackCandidates
# TextFileOutput: if set to 1, textfiles with hitcoordinates are produced, only usefull to plot and visualize some events
param_cdctracking = {
    'CDCSimHitsColName': 'CDCSimHits',
    'CDCHitsColName': 'CDCHits',
    'CDCRecoHitsColName': 'CDCRecoHits',
    'CDCTrackCandidatesColName': 'CDCTrackCandidates',
    'GFTrackCandidatesColName': 'GFTrackCandidates_PatternReco',
    'GFTrackCandToCDCRecoHitsColName': 'GFTrackCandidateToCDCRecoHits_PatternReco',
    'TextFileOutput': 0,
    }
cdctracking.param(param_cdctracking)

# Input collections: names for RecoHits and GFTrackCandidates
# Output collections: names for GFTracks and Tracks
# Here only CDCHits hits are available
param_cdcfitting = {
    'GFTrackCandidatesColName': 'GFTrackCandidates_PatternReco',
    'CDCRecoHitsColName': 'CDCRecoHits',
    'SVDRecoHitsColName': 'SVDRecoHits',
    'PXDRecoHitsColName': 'PXDRecoHits',
    'TracksColName': 'Tracks',
    'GFTracksColName': 'GFTracks',
    'UseCDCHits': 1,
    'UseSVDHits': 0,
    'UsePXDHits': 0,
    }

cdcfitting.param(param_cdcfitting)

output.param('outputFileName', 'PatternRecoTracks.root')

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
main.add_module(cdcrecohitmaker)

main.add_module(cdctracking)

# uncomment this module if you only want to look on pattern reco (without fitting)
main.add_module(cdcfitting)

main.add_module(output)

# Process events
process(main)
