#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This demonstrates that SVDClusterizer works without MC information.
# To do the test, first run
#    basf2 svd/examples/SVDTest.py
# to produce file SVDTestOutput.root. Then run this file,
#    basf2 svd/examples/SVDTestNoMC.py
# and check there are no runtime errors/warnings, and that the resulting
# output file SVDNoMCOutput.root contains the same cluster data as
# SVDTestOutput.root.

from basf2 import *

# show warnings during processing
set_log_level(LogLevel.WARNING)

# Register modules

# Data input from a file with only EventInfo and SVDDigits
input = register_module('RootInput')
input.param('inputFileName', 'SVDTestOutput.root')
input.param('branchNames', ['EventMetaData', 'SVDDigits'])
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# SVD clusterizer
SVDCLUST = register_module('SVDClusterizer')
# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'SVDNoMCOutput.root')

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('components', ['MagneticField', 'SVD'])

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(input)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(SVDCLUST)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
#
