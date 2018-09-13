#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from tracking import *
from simulation import add_simulation
from ROOT import Belle2

numEvents = 100

# first register the modules

set_random_seed(1)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.INFO

pxdROIFinder = register_module('PXDROIFinder')
pxdROIFinder.logging.log_level = LogLevel.DEBUG
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

# PXD digitization module
PXDDIGI = register_module('PXDDigitizer')
# PXD clusterizer
PXDCLUST = register_module('PXDClusterizer')

pxdClusterFilter = register_module('PXDclusterFilter')
pxdClusterFilter.param({'ROIidsName': 'ROIs', 'CreateOutside': True})

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main, components=['MagneticField', 'PXD', 'SVD', 'CDC'], usePXDDataReduction=False)
add_tracking_reconstruction(main, ['SVD', 'CDC'])
main.add_module(pxdROIFinder)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
main.add_module(pxdClusterFilter)
# display = register_module("Display")
# main.add_module(display)

main.add_module('RootOutput')

# Process events
process(main)

print(statistics)
