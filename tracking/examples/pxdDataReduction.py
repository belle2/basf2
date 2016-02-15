#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################################
#
# This steering file creates the Belle II detector geometry (magnetic field, SVD and PXD only),
# and perfoms the PXD Data Reduction based on the reconstruction of tracks from SVD-only.
#
# After registering the modules needed for the simulation of the detector, the
# simulation of the event and the reconstruction of the tracks (TrackFinderMCTruth)
# we add the PXD Data Reduction Module (PXDDataReduction) and
# the module that performs the analysis of the PXDDataReduction module performance
# (PXdDaraRedAnalysis)
#
##############################################################################

import os
from basf2 import *
from simulation import add_simulation
from beamparameters import add_beamparameters

numEvents = 10

# first register the modules

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')
gearbox = register_module('Gearbox')

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.INFO

geometry = register_module('Geometry')
# geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

track_finder_mc_truth = register_module('TrackFinderMCTruth')
track_finder_mc_truth.logging.log_level = LogLevel.INFO
# track_finder_mc_truth.logging.debug_level = 101

# select which detectors you would like to use
param_track_finder_mc_truth = {
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'MinimalNDF': 6,
    'UseClusters': 1,
    'WhichParticles': ['PXD', 'SVD'],
    'GFTrackCandidatesColName': 'mcTrackCands',
}
track_finder_mc_truth.param(param_track_finder_mc_truth)

# track fitting
setupGenfit = register_module('SetupGenfitExtrapolation')

trackfitter = register_module('GenFitter')
trackfitter.param({'BuildBelle2Tracks': False,
                   "GFTracksColName": 'mcTracks',
                   "PDGCodes": [211],
                   'GFTrackCandidatesColName': 'mcTrackCands'})
trackfitter.set_name('SVD-only GenFitter')


pxdDataRed = register_module('PXDDataReduction')
pxdDataRed.logging.log_level = LogLevel.INFO
param_pxdDataRed = {
    'gfTrackListName': 'mcTracks',
    'PXDInterceptListName': 'PXDIntercepts',
    'ROIListName': 'ROIs',
    'sigmaSystU': 0.02,
    'sigmaSystV': 0.02,
    'numSigmaTotU': 10,
    'numSigmaTotV': 10,
    'maxWidthU': 0.5,
    'maxWidthV': 0.5,
}
pxdDataRed.param(param_pxdDataRed)

pxdDataRedAnalysis = register_module('PXDDataRedAnalysis')
pxdDataRedAnalysis.logging.log_level = LogLevel.DEBUG
param_pxdDataRedAnalysis = {
    'gfTrackListName': 'mcTracks',
    'PXDInterceptListName': 'PXDIntercepts',
    'ROIListName': 'ROIs',
    'writeToRoot': True,
    'rootFileName': 'pxdDataRedAnalysis_SVDCDC_MCTF',
}
pxdDataRedAnalysis.param(param_pxdDataRedAnalysis)

rootOutput = register_module('RootOutput')

# Create paths
main = create_path()

beamparameters = add_beamparameters(main, "Y4S")
# beamparameters = add_beamparameters(main, "Y1S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)
# beamparameters.param("smearEnergy", False)
# print_params(beamparameters)

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(beamparameters)
main.add_module(evtgeninput)
add_simulation(main)
main.add_module(track_finder_mc_truth)
main.add_module(setupGenfit)
main.add_module(trackfitter)
main.add_module(pxdDataRed)
main.add_module(pxdDataRedAnalysis)
# main.add_module(rootOutput)

# Process events
process(main)

print(statistics)
