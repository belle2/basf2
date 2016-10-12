#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from tracking import *
from simulation import add_simulation
from ROOT import Belle2

reset_database()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)

numEvents = 5

# first register the modules

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.INFO

pxdDataRed = register_module('PXDDataReduction')
pxdDataRed.logging.log_level = LogLevel.DEBUG
pxdDataRed.logging.debug_level = 2
param_pxdDataRed = {
    'recoTrackListName': 'RecoTracks',
    'PXDInterceptListName': 'PXDIntercepts',
    'ROIListName': 'ROIs',
    'numIterKalmanFilter': 5,
    # 'tolerancePhi': 0.15,
    # 'toleranceZ': 0.5,
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
pxdDataRedAnalysis.logging.debug_level = 2
param_pxdDataRedAnalysis = {
    'recoTrackListName': 'RecoTracks',
    'PXDInterceptListName': 'PXDIntercepts',
    'ROIListName': 'ROIs',
    'writeToRoot': True,
    'rootFileName': 'pxdDataRedAnalysis_SVDCDC_MCTF_newVersion',
    }
pxdDataRedAnalysis.param(param_pxdDataRedAnalysis)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main)
# add_mc_tracking_reconstruction(main, ['SVD', 'CDC'], False)
add_tracking_reconstruction(main, ['SVD', 'CDC'], False)
main.add_module(pxdDataRed)
main.add_module(pxdDataRedAnalysis)

# Process events
process(main)

print(statistics)
