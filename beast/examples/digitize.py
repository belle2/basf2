#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from optparse import OptionParser

# --------------------------------------------------------------------
# Example steering script for digitization and analysis of existing
# background files containing only SimHits
# Analysis output histograms are stored into a root file
# In addition you can store the collection of hits in your detector
# by uncommenting root output module lines
#
# run as: basf2 beast/examples/digitize.py -- -f inputfile -o outputfile
# --------------------------------------------------------------------

parser = OptionParser()
parser.add_option('-f', '--file', dest='filename',
                  default='input.root')
parser.add_option('-o', '--output', dest='output_filename',
                  default='output.root')

(options, args) = parser.parse_args()

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# input root file
input_module = b2.register_module('RootInput')
input_module.param('inputFileName', options.filename)
main.add_module(input_module)

histo = b2.register_module('HistoManager')
histo.param('histoFileName', options.output_filename)  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# digitizers (for example He3Digitizer)
he3digi = b2.register_module('He3Digitizer')
main.add_module(he3digi)

# make histograms (for example for He3)
he3study = b2.register_module('He3tubeStudy')
main.add_module(he3study)

# root output module
# now the root output file contains digitized He3 hits (He3tubeHits)!
# output = register_module('RootOutput')
# output.param('outputFileName', options.output_filename)
# if you want to store only branches of interest (hits in your detector, etc.) use
# output.param('branchNames', ['He3tubeHits'])
# main.add_module(output)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
