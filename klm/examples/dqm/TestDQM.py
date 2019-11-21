#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# KLM data quality monitor test.
#
# DQM analysis example (the next stage of DQM data processing) is
# dqm/analysis/examples/DQMAnalysisKLM.py

import basf2
import sys
from daqdqm.commondqm import add_common_dqm

raw_data = False
if len(sys.argv) >= 2:
    if sys.argv[1] == 'raw':
        raw_data = True

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

# Unpacker and reconstruction.
if raw_data:
    main.add_module('KLMUnpacker')
    main.add_module('BKLMReconstructor')

# DQM
add_common_dqm(main, components=['KLM'])

# Process events
basf2.process(main)
print(basf2.statistics)
