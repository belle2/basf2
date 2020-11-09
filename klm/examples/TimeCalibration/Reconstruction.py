#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Reconstruction with EKLM time calibration data collection.

import sys
import basf2
from reconstruction import add_reconstruction, add_cdst_output
import sys

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

input = basf2.register_module('RootInput')
input.param('inputFileName', sys.argv[1])

gearbox = basf2.register_module('Gearbox')

# Create the main path and add the modules.
main = basf2.create_path()
main.add_module(input)
main.add_module(gearbox)
add_reconstruction(main)
add_cdst_output(main, filename=sys.argv[2], raw_format=True)
main.add_module('Progress')

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
