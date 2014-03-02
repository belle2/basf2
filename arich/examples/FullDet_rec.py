#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to analyse the output of FullDet_sim.py.
# It performs calculation of ARICH PID likelihood for each track, and stores
# information needed for PID efficiency calculation into output root file.
#
#
##############################################################################

from basf2 import *
from optparse import OptionParser

# Options from command line
parser = OptionParser()
parser.add_option('-f', '--file', dest='filename',
                  default='extArichTest_sim.root')
parser.add_option('-o', '--output', dest='output',
                  default='extArichTest_rec.root')
(options, args) = parser.parse_args()
input_fname = str(options.filename)
output_fname = str(options.output)

# suppress messages and warnings during processing DEBUG, INFO, WARNING, ERROR
set_log_level(LogLevel.INFO)

# input file module
input_module = register_module('RootInput')
input_module.param('inputFileName', input_fname)

# Show progress of processing
progress = register_module('Progress')

# Load parameters
gearbox = register_module('Gearbox')

# Create geometry
geometry = register_module('Geometry')
geometry.param('Components', ['ARICH'])  #    'MagneticField',
                                         #    'BeamPipe',
                                         #    'PXD',
                                         #    'SVD',
                                         #    'CDC',

# ARICH digitization module
arichDIGI = register_module('ARICHDigitizer')

# ARICH reconstruction module
arichRECO = register_module('ARICHReconstructor')
arichRECO.logging.log_level = LogLevel.DEBUG
arichRECO.logging.debug_level = 20
arichRECO.param('inputTrackType', 1)

# my module - reconstruction efficiency analysis
arichEfficiency = register_module('ARICHAnalysis')
arichEfficiency.logging.log_level = LogLevel.DEBUG
arichEfficiency.logging.debug_level = 20
arichEfficiency.param('outputFile', output_fname)

# Do the simulation
# =============================================================================
main = create_path()
main.add_module(input_module)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(arichDIGI)
main.add_module(arichRECO)
main.add_module(arichEfficiency)

# Process events
process(main)

# Print call statistics
print statistics
