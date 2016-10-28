#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Reconstruction of generated ee->mumu(ISR) events.

from basf2 import *
from simulation import *
import sys

# Set the global log level
set_log_level(LogLevel.INFO)

input = register_module('RootInput')
input.param('inputFileName', sys.argv[1])

output = register_module('RootOutput')
output.param('outputFileName', sys.argv[2])

# Create the main path and add the modules
main = create_path()
main.add_module(input)
add_simulation(main)
main.add_module('Progress')
main.add_module(output)

# generate events
process(main)

# show call statistics
print(statistics)
