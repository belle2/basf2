#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################

import os
from basf2 import *

# first register the modules

filename_data = '<PATH TO SIMULATION DATA>/PXDClusterSimulationData.root'

rootInputModule = register_module('RootInput')
rootInputModule.param({'inputFileName': filename_data})

gearboxModule = register_module('Gearbox')
geometryModule = register_module('Geometry')

track_finder_mc_truth = register_module('TrackFinderMCTruth')
track_finder_mc_truth.logging.log_level = LogLevel.ERROR
track_finder_mc_truth.logging.debug_level = 101

# select which detectors you would like to use
param_track_finder_mc_truth = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'MinimalNDF': 1,
    'UseClusters': 1,
    'WhichParticles': ['PXD', 'SVD'],
}
track_finder_mc_truth.param(param_track_finder_mc_truth)

# fitting
trackfitting = register_module('GenFitter')

# fit the tracks with one iteration of Kalman filter
# 'PDGCodes': [] means use the pdgCodes from
# the TrackFinderMCTruth wich are the correct ones
param_trackfitting = {  # 'PDGCodes': [],
    'StoreFailedTracks': 0,
    'FilterId': 'DAF',
    'ProbCut': 0.001,
    'UseClusters': True,
    'NMaxFailedHits': 5,
    'NMinIterations': 3,
    'NMaxIterations': 5,
    'GFTracksColName': 'gfTracks',
    'TracksColName': '',
}
trackfitting.param(param_trackfitting)
trackfitting.logging.log_level = LogLevel.WARNING

# ROI cluster rescue
PXDClusterRescueROI = register_module('PXDClusterRescueROI')
PXDClusterRescueROI.param({'gfTracksColName': 'gfTracks'})
PXDClusterRescueROI.param({'factorSigmaUV': 10})
PXDClusterRescueROI.param({'tolerancePhi': 0.3})
PXDClusterRescueROI.param({'toleranceZ': 0.1})
PXDClusterRescueROI.param({'maxROILength': 77})
PXDClusterRescueROI.logging.log_level = LogLevel.WARNING

PXDClusterRescueROIAnalysis = register_module('PXDClusterRescueROIAnalysis')
PXDClusterRescueROIAnalysis.logging.log_level = LogLevel.DEBUG
PXDClusterRescueROIAnalysis.param({'TracksColName': ''})
PXDClusterRescueROIAnalysis.param({'filenameLog': 'PXDClusterRescueROIAnalysis_signal.log'})
PXDClusterRescueROIAnalysis.param({'filenameRoot': 'PXDClusterRescueROIAnalysis_signal.root'})

# Output
eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

rootOutput = register_module('RootOutput')

# Create paths
main = create_path()

# Add modules to paths

main.add_module(rootInputModule)
main.add_module(gearboxModule)
main.add_module(geometryModule)

main.add_module(track_finder_mc_truth)
main.add_module(trackfitting)
main.add_module(PXDClusterRescueROI)
main.add_module(PXDClusterRescueROIAnalysis)
# main.add_module(rootOutput)

# Process events
process(main)

print(statistics)
