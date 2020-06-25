#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from tracking import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2

numEvents = 10000

set_random_seed(1)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.INFO

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main)
add_reconstruction(main)

fil = register_module('SVDShaperDigitsFromTracks')
fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
main.add_module(fil)

input_branches = [
    'SVDShaperDigitsFromTracks',
    'EventT0',
    'SVDShaperDigits'
]

main.add_module("RootOutput", branchNames=input_branches, outputFileName="RootOutput_CoGCalibration_10k.root")

print_path(main)

# Process events
process(main)

print(statistics)
