#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Reconstruction of generated ee->mumu(ISR) events.

import sys
import basf2
from simulation import add_simulation

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

input = basf2.register_module('RootInput')
input.param('inputFileName', sys.argv[1])

output = basf2.register_module('RootOutput')
output.param('outputFileName', sys.argv[2])

# Create the main path and add the modules
main = basf2.create_path()
main.add_module(input)
add_simulation(main)
main.add_module('Progress')
main.add_module(output)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
