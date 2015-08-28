#!/usr/bin/env python
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
    'UseCDCHits': 1,
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
    'TracksColName': 'Tracks',
}
trackfitting.param(param_trackfitting)
trackfitting.logging.log_level = LogLevel.WARNING

# ROI cluster rescue
PXDClusterRescueROI = register_module('PXDClusterRescueROI')
PXDClusterRescueROI.param({'gfTracksColName': 'gfTracks'})
PXDClusterRescueROI.param({'factorSigmaUV': 5})
PXDClusterRescueROI.param({'tolerancePhi': 0.15})
PXDClusterRescueROI.param({'toleranceZ': 0.5})
PXDClusterRescueROI.param({'maxROILengthU': 140})
PXDClusterRescueROI.param({'maxROILengthV': 140})
PXDClusterRescueROI.logging.log_level = LogLevel.WARNING

# NN cluster rescue
PXDClusterRescueNN = register_module('PXDClusterRescueNN')
PXDClusterRescueNN.param({'filenameExpertise': '../data/PXDClusterNeuroBayes.nb'})
PXDClusterRescueNN.param({'classThreshold': 0.568627})
PXDClusterRescueNN.logging.log_level = LogLevel.WARNING

# cluster rescue analysis
PXDClusterRescueAnalysis = register_module('PXDClusterRescueAnalysis')
PXDClusterRescueAnalysis.logging.log_level = LogLevel.WARNING
PXDClusterRescueAnalysis.param({'TracksColName': 'Tracks'})
PXDClusterRescueAnalysis.param({'filenameLog': 'PXDClusterRescueAnalysis.log'})
PXDClusterRescueAnalysis.param({'filenameRoot': 'PXDClusterRescueAnalysis.root'})
PXDClusterRescueAnalysis.param({'particlePtThreshold': 0.065})
PXDClusterRescueAnalysis.logging.log_level = LogLevel.DEBUG

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
main.add_module(PXDClusterRescueNN)
main.add_module(PXDClusterRescueAnalysis)
# main.add_module(rootOutput)

# Process events
process(main)

print statistics
