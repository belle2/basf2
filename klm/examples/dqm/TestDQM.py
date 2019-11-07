#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# KLM data quality monitor test.
#
# DQM analysis example (the next stage of DQM data processing) is
# dqm/analysis/examples/DQMAnalysisKLM.py

import basf2
from daqdqm.commondqm import add_common_dqm

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

# Create path
main = basf2.Path()

# Input
main.add_module('RootInput')

# Progress
main.add_module('Progress')

# Gearbox
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

# Histogram manager
main.add_module('HistoManager',
                histoFileName='KLMDQM.root')

# Unpacker
main.add_module('KLMUnpacker')

# BKLM reconstructor
main.add_module('BKLMReconstructor')

# DQM
add_common_dqm(main, components=['KLM'])

# Process events
basf2.process(main)
print(basf2.statistics)
