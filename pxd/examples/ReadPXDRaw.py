#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

set_log_level(LogLevel.WARNING)

# Register modules
input = register_module('RootInput')
input.param('inputFileName', '-path-to-/PXDRawHit.root')
# Don't want anything but raw hits from the file
input.param('branchNames', ['PXDRawHits'])


# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'PXD-histo.root')  # File to save histograms

# Report progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# Testbeam geometry, actually any that supports sensor 1.1.1. will do
gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v1.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
geometry.param('components', ['TB'])

# PXDRawHitSorter
PXDRaw = register_module('PXDRawHitSorter')
PXDRaw.param('mergeFrames', False)
PXDRaw.param('zeroSuppressionCut', 1)
PXDRaw.param('mergeDuplicates', True)
# We need to trick basf2 into thinking that data come from a plane in geometry.
PXDRaw.param('acceptFake', True)
# PXD clusterizer
PXDClust = register_module('PXDClusterizer')
PXDClust.param('TanLorentz', 0.)


# PXD DQM module
PXDDQM = register_module('PXDDQM')

#output
output = register_module('RootOutput')
output.param('outputFileName', 'runPXDoutput.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(input)
main.add_module(histo)  # immediately after master module
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(PXDRaw)
main.add_module(PXDClust)
main.add_module(PXDDQM)
main.add_module(output)

# Process events
process(main)

# Print statistics
print statistics
