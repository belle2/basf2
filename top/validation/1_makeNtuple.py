#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# ---------------------------------------------------------------
# Make a flat ntuple for validation of top package
# ---------------------------------------------------------------

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Input
roinput = register_module('RootInput')
roinput.param('inputFileNames', ['../EvtGenSimRec.root',
              '../EvtGenSimRec_B2Kpi.root'])
main.add_module(roinput)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP is needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Output: make flat ntuple from TOPLikelihoods, tracking info and MC truth
output = register_module('TOPNtuple')
output.param('outputFileName', '../TOPNtuple.root')
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics

