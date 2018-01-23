#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# EKLM data quality monitor test.
# Use ParticleGun/FullReconstruction.py to generate the input data.

from basf2 import *
from ROOT import Belle2
import sys

# Set the global log level
set_log_level(LogLevel.INFO)

# Input
input = register_module('RootInput')
input.param('inputFileName', 'ParticleGunMuonsFull.root')

# Histogram creation
histo = register_module('HistoManager')
histo.param('histoFileName', 'EKLMDQM.root')

# DQM
eklm_dqm = register_module('EKLMDQM')

# Create paths
main = create_path()
main.add_module(input)
main.add_module(histo)
main.add_module(eklm_dqm)

# Process events
process(main)
print(statistics)
