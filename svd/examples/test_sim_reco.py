#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from tracking import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2

numEvents = 2000

# first register the modules

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
add_simulation(main, components=['MagneticField', 'SVD'], usePXDDataReduction=False)
add_reconstruction(main, components=['MagneticField', 'SVD'])

# display = register_module("Display")
# main.add_module(display)

main.add_module('RootOutput')

# Process events
process(main)

print(statistics)
