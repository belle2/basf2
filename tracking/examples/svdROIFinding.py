#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from tracking import add_prefilter_tracking_reconstruction
from simulation import add_simulation

b2.set_random_seed(1)

# Create paths
main = b2.create_path()

# Add modules to paths
main.add_module('EventInfoSetter', expList=[0], runList=[1], evtNumList=[100])
main.add_module("EvtGenInput")
add_simulation(main, forceSetPXDDataReduction=True, usePXDDataReduction=False)
add_prefilter_tracking_reconstruction(main, ['CDC'], mcTrackFinding=True)
# Add the SVDROIFinder module
main.add_module(
    'SVDROIFinder',
    recoTrackListName='RecoTracks',
    SVDInterceptListName='SVDIntercepts',
    ROIListName='ROIs',
    tolerancePhi=0.15,
    toleranceZ=0.5,
    sigmaSystU=0.02,
    sigmaSystV=0.02,
    numSigmaTotU=10,
    numSigmaTotV=10,
    maxWidthU=0.5,
    maxWidthV=0.5,
    logLevel=b2.LogLevel.DEBUG)
# Add the corresponding analysis module if desired
# main.add_module(
#     'SVDROIFinderAnalysis',
#     recoTrackListName='RecoTracks',
#     SVDInterceptListName='SVDIntercepts',
#     ROIListName='ROIs',
#     writeToRoot=True,
#     rootFileName='svdDataRedAnalysis_SVDCDC_MCTF_test',
#     logLevel=b2.LogLevel.RESULT)

main.add_module('Progress')

# Process events
b2.process(main)

print(b2.statistics)
