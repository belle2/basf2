#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# KLM data quality monitor test.

import basf2
from daqdqm.commondqm import add_common_dqm
import sys

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

# Input
input = basf2.register_module('RootInput')
input.param('inputFileName', sys.argv[1])

# Histogram creation
histo = basf2.register_module('HistoManager')
histo.param('histoFileName', 'KLMDQM.root')

# Gearbox
gearbox = basf2.register_module('Gearbox')

# Geometry
geometry = basf2.register_module('Geometry')

# Create paths
main = basf2.create_path()
main.add_module(input)
main.add_module(histo)
main.add_module(gearbox)
main.add_module(geometry)
add_common_dqm(main, components=['EKLM', 'BKLM'])

# Process events
basf2.process(main)
print(basf2.statistics)
