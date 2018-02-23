#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Script for ARICH cosmics data simple analysis
#
# run as: "basf2 ARICHCosmicTestData.py -- -f path2datafile.sroot"
# add "-s 1" for event display
#
# By: Luka Santelj

from basf2 import *
import os
from optparse import OptionParser

home = os.environ['BELLE2_LOCAL_DIR']

# parameters
parser = OptionParser()
parser.add_option('-f', '--file', dest='filename', default='/ghi/fs01/belle2/bdata/users/tkonno/cosmic/cosmic.0002.00719.HLT3.root')
parser.add_option('-o', '--output', dest='output', default='ARICHHits.root')
parser.add_option('-d', '--debug', dest='debug', default=0)
parser.add_option('-s', '--display', dest='display', default=0)
(options, args) = parser.parse_args()

reset_database()
use_central_database("development")

# create paths
main = create_path()

# root input module
input_module = register_module('RootInput')
input_module.param('inputFileName', options.filename)
main.add_module(input_module)

# Histogram manager module
# histo = register_module('HistoManager')
# histo.param('histoFileName', "histograms.root")  # File to save histograms
# main.add_module(histo)


# build geometry if display option
if int(options.display):
    gearbox = register_module('Gearbox')
    geometry = register_module('Geometry')
    geometry.param('components', ['ARICH'])
#   geometry.param('useDB', 1)
    main.add_module(gearbox)
    main.add_module(geometry)

# unpack raw data
unPacker = register_module('ARICHUnpacker')
unPacker.param('debug', int(options.debug))
main.add_module(unPacker)

# create ARICHHits from ARICHDigits
arichHits = register_module('ARICHFillHits')
main.add_module(arichHits)

# create simple DQM histograms
# arichHists = register_module('ARICHDQM')
# main.add_module(arichHists)

# uncomment to store dataobjects
output = register_module('RootOutput')
output.param('outputFileName', options.output)
output.param('branchNames', ['ARICHDigits', 'ARICHHits'])
main.add_module(output)

# add display module if display option
if int(options.display):
    display = register_module('Display')
    # show arich hits
    display.param('showARICHHits', True)
    # show full geometry
    display.param('fullGeometry', True)
    main.add_module(display)

# show progress
progress = register_module('Progress')
main.add_module(progress)

# process
process(main)

# print stats
print(statistics)
