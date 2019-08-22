#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Reconstruction with EKLM alignment along strips data collection.

from basf2 import *
from reconstruction import *
import sys

# Set the global log level
set_log_level(LogLevel.INFO)

input = register_module('RootInput')
input.param('inputFileName', sys.argv[1])

gearbox = register_module('Gearbox')

eklm_alignment_along_strips = register_module('EKLMAlignmentAlongStripsCollector')

# Create the main path and add the modules
main = create_path()
main.add_module(input)
main.add_module("HistoManager", histoFileName=sys.argv[2])
main.add_module(gearbox)
add_reconstruction(main)
main.add_module(eklm_alignment_along_strips)

# generate events
process(main)

# show call statistics
print(statistics)
