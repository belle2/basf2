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
from simulation import add_simulation
from reconstruction import add_reconstruction

numEvents = 10000

b2.set_random_seed(1)

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = b2.register_module('EventInfoPrinter')

evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.logging.log_level = b2.LogLevel.INFO

# Create paths
main = b2.create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtgeninput)
add_simulation(main)
add_reconstruction(main)

fil = b2.register_module('SVDShaperDigitsFromTracks')
fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
main.add_module(fil)

input_branches = [
    'SVDShaperDigitsFromTracks',
    'EventT0',
    'SVDShaperDigits'
]

main.add_module("RootOutput", branchNames=input_branches, outputFileName="RootOutput_CoGCalibration_10k.root")

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
