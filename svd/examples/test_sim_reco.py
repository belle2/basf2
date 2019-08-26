#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from tracking import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2

numEvents = 10000

# CoG calibration - CAF - BEGIN
use_database_chain()
# no cuts on cluster time
# use_central_database("svd_testGT_noSVDHitTimeSelection")
# no time in the sectormaps:
use_local_database("centraldb/dbcache.txt", "centraldb", invertLogging=True)
# CoG calibration - CAF - END

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

# CoG calibration - CAF - BEGIN
fil = register_module('SVDShaperDigitsFromTracks')
fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
main.add_module(fil)

input_branches = [
    'SVDShaperDigitsFromTracks',
    'EventT0',
    'SVDShaperDigits'
]

main.add_module("RootOutput", branchNames=input_branches, outputFileName="RootOutput_CoGCalibration_10k.root")

# CoG calibration - CAF - END

# display = register_module("Display")
# main.add_module(display)
# main.add_module('RootOutput')

print_path(main)

# Process events
process(main)

print(statistics)
