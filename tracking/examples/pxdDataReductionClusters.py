#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

numEvents = 100

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

# PXD digitization module
PXDDIGI = b2.register_module('PXDDigitizer')
# PXD clusterizer
PXDCLUST = b2.register_module('PXDClusterizer')

pxdClusterFilter = b2.register_module('PXDclusterFilter')
pxdClusterFilter.param({'ROIidsName': 'ROIs', 'CreateOutside': True, 'overrideDB': True, 'enableFiltering': True})

# Create paths
main = b2.create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main, components=['PXD', 'SVD', 'CDC'], forceSetPXDDataReduction=True, usePXDDataReduction=False)
add_tracking_reconstruction(main, ['SVD', 'CDC'])
main.add_module(pxdROIFinder)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
main.add_module(pxdClusterFilter)
# display = register_module("Display")
# main.add_module(display)

main.add_module('RootOutput')

# Process events
b2.process(main)

print(b2.statistics)
