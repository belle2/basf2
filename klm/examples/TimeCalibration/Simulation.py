#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Reconstruction of generated ee->mumu(ISR) events.

import sys
import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_cdst_output

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

input = basf2.register_module('RootInput')
input.param('inputFileName', sys.argv[1])

# Create the main path and add the modules
main = basf2.create_path()
main.add_module(input)
add_simulation(main)
add_reconstruction(main, reconstruct_cdst='rawFormat')
add_cdst_output(main, filename=sys.argv[2], rawFormat=True)
main.add_module('Progress')

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
