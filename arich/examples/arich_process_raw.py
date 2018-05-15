#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example script for processing of raw data
# It converts arich raw data to arichDigits and arichHits
# Using "-t 1" option the cdc tracks are also reconstructed and stored into
# the output root file (along with the arichHits).
# Using option "-a 1" only the data from events that contain reconstruced track hit
# in the is included into the output file.
# ARICHDQM module is also included, it produces "histograms.root" with some basic
# dqm histograms (using all events). You can conveniently display some of them using
# "root -l histograms.root arich/utility/scripts/plotDQM.C"
#
# run as: "basf2 arich/examples/arich_process_raw.py -- -f path2datafile.root"
# additional options:
# "-d 1" for unpacker debug info (data headers are printed, >1 raw data bits are printed)
# "-t 1" include CDC track reconstruction (reconstructed tracks are stored into output )
# "-a 1" store only data from events with track hit in arich
#
# Author: Luka Santelj

from basf2 import *
import os
from optparse import OptionParser
from reconstruction import add_cosmics_reconstruction
home = os.environ['BELLE2_LOCAL_DIR']

# set the tag for the global cosmic test (2018 Feb,Mar)
reset_database()
use_central_database("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000")

# parameters
parser = OptionParser()
parser.add_option(
    '-f',
    '--file',
    dest='filename',
    default='/ghi/fs01/belle2/bdata/users/tkonno/cosmic/cosmic.0002.00951.HLT3.f00000.root')
parser.add_option('-o', '--output', dest='output', default='ARICHHits.root')
parser.add_option('-d', '--debug', dest='debug', default=0)
parser.add_option('-t', '--tracking', dest='tracking', default=0)
parser.add_option('-a', '--arichtrk', dest='arichtrk', default=0)
(options, args) = parser.parse_args()

# create paths
main = create_path()
store = create_path()

# root input module
input_module = register_module('RootInput')
input_module.param('inputFileName', options.filename)
# input_module.param('entrySequences',['5100:5300']) # process only range of events
main.add_module(input_module)

# Histogram manager module
histo = register_module('HistoManager')
histo.param('histoFileName', "DQMhistograms.root")  # File to save histograms
main.add_module(histo)

# build geometry if display option
if int(options.tracking):
    gearbox = register_module('Gearbox')
    main.add_module(gearbox)
    geometry = register_module('Geometry')
    geometry.param('useDB', 1)
    main.add_module(geometry)

# unpack raw data
unPacker = register_module('ARICHUnpacker')
unPacker.param('debug', int(options.debug))
main.add_module(unPacker)

# create ARICHHits from ARICHDigits
arichHits = register_module('ARICHFillHits')
# set bitmask for makin hits form digits
arichHits.param("bitMask", 0xFF)
main.add_module(arichHits)

if int(options.tracking):
    cdcunpacker = register_module('CDCUnpacker')
    cdcunpacker.param('xmlMapFileName', "cdc/data/ch_map.dat")
    cdcunpacker.param('enablePrintOut', False)
    main.add_module(cdcunpacker)
    add_cosmics_reconstruction(main, 'CDC', False)

# create simple DQM histograms
arichHists = register_module('ARICHDQM')
main.add_module(arichHists)

# store the dataobjects
output = register_module('RootOutput')
output.param('outputFileName', options.output)
branches = ['ARICHDigits', 'ARICHHits', 'ARICHInfo']
if int(options.tracking):
    branches.extend(['Tracks', 'TrackFitResults', 'RecoTracks', 'RecoHitInformations', 'ExtHits'])
output.param('branchNames', branches)
if int(options.arichtrk):
    store.add_module(output)
else:
    main.add_module(output)

# show progress
progress = register_module('Progress')
main.add_module(progress)

if int(options.arichtrk):
    arichHists.if_value('==1', store)

# process
process(main)

# print stats
print(statistics)
