#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys

# ------------------------------------------------------------------------------
# This example shows how to do the beam alignment
#
# User has to modify at least the input file name below
# ------------------------------------------------------------------------------

inputFile = 'lepsdata/exp001_merge.root'  # file with the "top" tree
scan = {
    'scanY': [0.0, 0.1, 10],
    'scanZ': [125.0, 0.1, 10],
    'scanTheta': [90.0, 0.05, 10],
    'scanPhi': [0.0, 0.05, 10],
    'scanT0': 20,
    }
t0 = 1.5  # offset added to photon times when reading ntuple
bkg = 4  # number of background hits per event
pdfRange = {'tMin': 10.0, 'tMax': 55.0, 'numBins': 900}
iterations = 1
outputFile = 'beamAlignment.root'  # some control histograms

if not os.path.exists(inputFile):
    print '*** Input file ' + inputFile + ' not found'
    sys.exit(1)

# Input
lepsInput = register_module('TOPLeps2013Input')
lepsInput.param('inputFileName', inputFile)
lepsInput.param('t0', t0)
print_params(lepsInput)

# Gearbox
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/top/LEPS2013-6/TOP-CFD.xml')  # for CFD runs
# gearbox.param('fileName', 'testbeam/top/LEPS2013-6/TOP.xml') # for IRS3B runs

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])

# Beam alignment
beamAlignment = register_module('TOPbeamAlignment')
beamAlignment.param(scan)
beamAlignment.param('numIterations', iterations)
beamAlignment.param(pdfRange)
beamAlignment.param('minBkgPerBar', bkg)
beamAlignment.param('outputFileName', outputFile)
print_params(beamAlignment)

# Show progress of processing
progress = register_module('Progress')

# suppress info messages during processing:
set_log_level(LogLevel.WARNING)

# Create path
main = create_path()
main.add_module(lepsInput)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(beamAlignment)
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics

