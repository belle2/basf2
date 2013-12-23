#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys

# ------------------------------------------------------------------------------
# This example shows how to run a beta scan to measure the resolution in beta
#
# User has to modify at least input file name below
# ------------------------------------------------------------------------------

inputFile = 'lepsdata/exp001_merge.root'  # file with the "top" tree
outputFile = 'betaScan.root'  # histogram of reconstructed beta

if not os.path.exists(inputFile):
    print '*** Input file ' + inputFile + ' not found'
    sys.exit(1)

# Input
lepsInput = register_module('TOPLeps2013Input')
lepsInput.param('inputFileName', inputFile)
beam = {  # for CFD runs of exp001
    'x0': 0,
    'y0': -0.20,
    'z0': 124.30,
    'p': 2.12,
    'theta': 89.639,
    'phi': 0.057,
    }
lepsInput.param(beam)
lepsInput.param('t0', 1.51)  # for CFD runs of exp001
# print_params(lepsInput)

# Gearbox
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/top/LEPS2013-6/TOP-CFD.xml')  # for CFD runs
# gearbox.param('fileName', 'testbeam/top/LEPS2013-6/TOP.xml') # for IRS3B runs

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])

# Beta scan
betaScan = register_module('TOPbetaScan')
betaScan.param('betaMin', 0.95)
betaScan.param('betaMax', 1.05)
betaScan.param('maxTime', 55)  # use photons in the range 0 < t < 55 ns
betaScan.param('electronicJitter', 35e-3)
betaScan.param('minBkgPerBar', 5)
betaScan.param('outputFileName', outputFile)
# print_params(betaScan)

# Show progress of processing
progress = register_module('Progress')

# suppress info messages during processing:
set_log_level(LogLevel.WARNING)

# Create path
main = create_path()
main.add_module(lepsInput)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(betaScan)
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics

