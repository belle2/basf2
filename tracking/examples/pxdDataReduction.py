#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

pxdROIFinder = b2.register_module('PXDROIFinder')
pxdROIFinder.logging.log_level = b2.LogLevel.DEBUG
# pxdROIFinder.logging.debug_level = 2
param_pxdROIFinder = {
    'recoTrackListName': 'RecoTracks',
    'PXDInterceptListName': 'PXDIntercepts',
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
pxdROIFinder.param(param_pxdROIFinder)

pxdROIFinderAnalysis = b2.register_module('PXDROIFinderAnalysis')
pxdROIFinderAnalysis.logging.log_level = b2.LogLevel.RESULT
pxdROIFinderAnalysis.logging.debug_level = 1
param_pxdROIFinderAnalysis = {
    'recoTrackListName': 'RecoTracks',
    'PXDInterceptListName': 'PXDIntercepts',
    'ROIListName': 'ROIs',
    'writeToRoot': True,
    'rootFileName': 'pxdDataRedAnalysis_SVDCDC_MCTF_test',
}
pxdROIFinderAnalysis.param(param_pxdROIFinderAnalysis)

# Create paths
main = b2.create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main, components=['SVD', 'CDC'], forceSetPXDDataReduction=True, usePXDDataReduction=False)
add_tracking_reconstruction(main, ['SVD', 'CDC'], mcTrackFinding=True)
main.add_module(pxdROIFinder)
main.add_module(pxdROIFinderAnalysis)
# display = register_module("Display")
# main.add_module(display)

# Process events
b2.process(main)

print(b2.statistics)
