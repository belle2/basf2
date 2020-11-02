#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Reconstruction with EKLM alignment along strips data collection.

import basf2 as b2
from reconstruction import add_reconstruction
import sys

# Set the global log level
b2.set_log_level(b2.LogLevel.INFO)

input = b2.register_module('RootInput')
input.param('inputFileName', sys.argv[1])

gearbox = b2.register_module('Gearbox')

eklm_alignment_along_strips = b2.register_module('EKLMAlignmentAlongStripsCollector')

# Create the main path and add the modules
main = b2.create_path()
main.add_module(input)
main.add_module("HistoManager", histoFileName=sys.argv[2])
main.add_module(gearbox)
add_reconstruction(main)
main.add_module(eklm_alignment_along_strips)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
