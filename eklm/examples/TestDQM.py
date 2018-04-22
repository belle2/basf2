#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# EKLM data quality monitor test.
# Use ParticleGun/FullReconstruction.py to generate the input data.

from basf2 import *
from daqdqm.commondqm import *
import sys

# Set the global log level
set_log_level(LogLevel.INFO)

# Input
input = register_module('RootInput')
input.param('inputFileName', 'ParticleGunMuonsFull.root')

# Histogram creation
histo = register_module('HistoManager')
histo.param('histoFileName', 'EKLMDQM.root')

# Gearbox
gearbox = register_module('Gearbox')

# Geometry
geometry = register_module('Geometry')

# Create paths
main = create_path()
main.add_module(input)
main.add_module(histo)
main.add_module(gearbox)
main.add_module(geometry)
add_common_dqm(main, components=['EKLM'])

# Process events
process(main)
print(statistics)
