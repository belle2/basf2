#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################################
#
# This steering file creates the Belle II detector geometry, and perfoms:
# - TrackCand finding based on SVD clusters only using VXDTF (or MC track finder)
# - PXD Data Reduction
# - Filtering of the PXDDigits
#     - PXDDigitsIN contains the pixels contained in an ROIs
#     - PXDDigitsOUT contains the pixels NOT contained in an ROIs
# - Clusterization of the PXDDigitsIN
# - TrackCand finding based on SVD clusters and PXD clusters (produced one step before)
#
##############################################################################

import os
from basf2 import *
from simulation import add_simulation

seed = 1
numEvents = 100

useMCTrackCandFinder = True
useMCTrackCandFinder = False

set_random_seed(seed)

## register the modules

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')
gearbox = register_module('Gearbox')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.INFO

trackCandName_pre = 'vxdtftracks_pre'
trackCandName_post = 'vxdtftracks_post'
if useMCTrackCandFinder:
    trackCandName_pre = 'mcTracks'
    trackCandName_post = 'mcTracks'

track_finder_mc_truth = register_module('TrackFinderMCTruth')
track_finder_mc_truth.logging.log_level = LogLevel.INFO
param_track_finder_mc_truth = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'MinimalNDF': 1,
    'UseClusters': 1,
    'WhichParticles': ['PXD', 'SVD'],
    'GFTrackCandidatesColName': 'mcTracks',
    }
track_finder_mc_truth.param(param_track_finder_mc_truth)

# VXD TF (SVD) before PXD Data Reduction (pre ROI)
vxd_trackfinder_pre = register_module('VXDTF')
vxd_trackfinder_pre.param('GFTrackCandidatesColName', 'vxdtftracks_pre')
vxd_trackfinder_pre.param('TESTERexpandedTestingRoutines', False)
# settings from VXDTFModuleDemo.py (check)
vxd_trackfinder_pre.param('sectorSetup',
                          ['secMapEvtGenOnR10933June2014SVDStd-moreThan500MeV_SVD'
                          ,
                          'secMapEvtGenOnR10933June2014SVDStd-125to500MeV_SVD'
                          , 'secMapEvtGenOnR10933June2014SVDStd-30to125MeV_SVD'
                          ])
vxd_trackfinder_pre.param('tuneCutoffs', 0.06)

# PXD Data Reduction (ROI production)
pxdDataRed = register_module('PXDDataReduction')
pxdDataRed.logging.log_level = LogLevel.INFO
param_pxdDataRed = {
    'trackCandCollName': trackCandName_pre,
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

# Filtering of PXDDigits
pxd_digifilter = register_module('PXDdigiFilter')
pxd_digifilter.param('ROIidsName', 'ROIs')
pxd_digifilter.param('PXDDigitsInsideROIName', 'PXDDigitsIN')
pxd_digifilter.param('PXDDigitsOutsideROIName', 'PXDDigitsOUT')

# post filtering PXDDigits clusterization
pxd_clusterizer_post = register_module('PXDClusterizer')
pxd_clusterizer_post.param('Digits', 'PXDDigitsIN')
pxd_clusterizer_post.param('Clusters', 'PXDFilteredClustersName')

# VXD TF (SVD+PXD) after PXD Data Reduction (post ROI)
vxd_trackfinder_post = register_module('VXDTF')
vxd_trackfinder_post.param('GFTrackCandidatesColName', 'vxdtftracks_post')
vxd_trackfinder_post.param('TESTERexpandedTestingRoutines', False)
vxd_trackfinder_post.param('pxdClustersName', 'PXDFilteredClustersName')
# settings from VXDTFModuleDemo.py (check)
vxd_trackfinder_post.param('sectorSetup',
                           ['secMapEvtGenOnR10933June2014VXDStd-moreThan500MeV_PXDSVD'
                           ,
                           'secMapEvtGenOnR10933June2014VXDStd-125to500MeV_PXDSVD'
                           ,
                           'secMapEvtGenOnR10933June2014VXDStd-30to125MeV_PXDSVD'
                           ])
vxd_trackfinder_post.param('tuneCutoffs', 0.22)

rootOutput = register_module('RootOutput')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main)

if useMCTrackCandFinder:
    main.add_module(track_finder_mc_truth)
else:
    main.add_module(vxd_trackfinder_pre)

main.add_module(pxdDataRed)
main.add_module(pxd_digifilter)
main.add_module(pxd_clusterizer_post)
main.add_module(vxd_trackfinder_post)
main.add_module(rootOutput)

# Process events
process(main)

print statistics
