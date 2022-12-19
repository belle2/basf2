#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from tracking import add_tracking_reconstruction
from simulation import add_simulation

numEvents = 2000

# first register the modules

b2.set_random_seed(1)

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = b2.register_module('EventInfoPrinter')

evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.logging.log_level = b2.LogLevel.INFO

svdROIFinder = b2.register_module('SVDROIFinder')
svdROIFinder.logging.log_level = b2.LogLevel.DEBUG
# svdROIFinder.logging.debug_level = 2
param_svdROIFinder = {
    'recoTrackListName': 'RecoTracks',
    'SVDInterceptListName': 'SVDIntercepts',
    'ROIListName': 'ROIs',
    'tolerancePhi': 0.15,
    'toleranceZ': 0.5,
    # optimized performance
    #    'sigmaSystU': 0.1,
    #    'sigmaSystV': 0.1,
    #    'numSigmaTotU': 10,
    #    'numSigmaTotV': 10,
    #    'maxWidthU': 2,
    #    'maxWidthV': 6,
    # official simulation
    'sigmaSystU': 0.02,
    'sigmaSystV': 0.02,
    'numSigmaTotU': 10,
    'numSigmaTotV': 10,
    'maxWidthU': 0.5,
    'maxWidthV': 0.5,
}
svdROIFinder.param(param_svdROIFinder)

svdROIFinderAnalysis = b2.register_module('SVDROIFinderAnalysis')
svdROIFinderAnalysis.logging.log_level = b2.LogLevel.RESULT
svdROIFinderAnalysis.logging.debug_level = 1
param_svdROIFinderAnalysis = {
    'recoTrackListName': 'RecoTracks',
    'SVDInterceptListName': 'SVDIntercepts',
    'ROIListName': 'ROIs',
    'writeToRoot': True,
    'rootFileName': 'svdDataRedAnalysis_SVDCDC_MCTF_test',
}
svdROIFinderAnalysis.param(param_svdROIFinderAnalysis)

# Create paths
main = b2.create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main, components=['MagneticField', 'SVD', 'CDC'], usePXDDataReduction=False)
add_tracking_reconstruction(main, ['CDC'], mcTrackFinding=True)
main.add_module(svdROIFinder)
# main.add_module(svdROIFinderAnalysis)
# display = register_module("Display")
# main.add_module(display)

# Process events
b2.process(main)

print(b2.statistics)
