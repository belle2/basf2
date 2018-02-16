#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from optparse import OptionParser

# --------------------------------------------------------------------
# Example steering script for digitization and analysis of existing
# background files containing only SimHits
# Analysis output histograms are stored into a root file
# In addition you can store the collection of hits in your detector
# by uncommenting root output module lines
#
# run as: basf2 beast/examples/digitize.py -- -f inputfile -o outputfile
#
# Author: Luka Santelj
# Date: 15.2.2018
# --------------------------------------------------------------------

parser = OptionParser()
parser.add_option('-f', '--file', dest='filename',
                  default='input.root')
parser.add_option('-o', '--output', dest='output_filename',
                  default='output.root')

(options, args) = parser.parse_args()

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# input root file
input_module = register_module('RootInput')
input_module.param('inputFileName', options.filename)
main.add_module(input_module)

histo = register_module('HistoManager')
histo.param('histoFileName', options.output_filename)  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# digitizers (for example He3Digitizer)
he3digi = register_module('He3Digitizer')
main.add_module(he3digi)

# make histograms (for example for He3)
he3study = register_module('He3tubeStudy')
main.add_module(he3study)

# root output module
# now the root output file contains digitized He3 hits (He3tubeHits)!
# output = register_module('RootOutput')
# output.param('outputFileName', options.output_filename)
# if you want to store only branches of interest (hits in your detector, etc.) use
# output.param('branchNames', ['He3tubeHits'])
# main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
