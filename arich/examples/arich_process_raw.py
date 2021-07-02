#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example script for processing of raw data
# It converts arich raw data to arichDigits and arichHits
# Using "-t 1" option the cdc tracks are also reconstructed and stored into
# the output root file (along with the arichHits).
# Using option "-a 1" only the data from events that contain reconstruced track hit
# in the is included into the output file.
# ARICHDQM module is also included, it produces "DQMhistograms.root" with some basic
# dqm histograms (using all events). You can conveniently display some of them using
# "root -l histograms.root arich/utility/scripts/plotDQM.C"
#
# NOTE: by default there is no output file made, only file with histograms from DQM
#       use option "-o filename" to make output root file with hits...
#
# run as: "basf2 arich/examples/arich_process_raw.py -- -f path2datafile.root"
# additional options:
# "-d 1" for unpacker debug info (data headers are printed, >1 raw data bits are printed)
# "-t 1" include CDC track reconstruction (reconstructed tracks are stored into output )
# "-a 1" store only data from events with track hit in arich
# "-g 1" include GDL unpacker to have TRGSummary
#
#
# Author: Luka Santelj

import basf2 as b2
import os
from optparse import OptionParser
from reconstruction import add_cosmics_reconstruction
home = os.environ['BELLE2_LOCAL_DIR']


# parameters
parser = OptionParser()
parser.add_option(
    '-f',
    '--file',
    dest='filename',
    default='/ghi/fs01/belle2/bdata/users/tkonno/cosmic/cosmic.0002.00951.HLT3.f00000.root')
parser.add_option('-o', '--output', dest='output', default='')
parser.add_option('-d', '--debug', dest='debug', default=0)
parser.add_option('-t', '--tracking', dest='tracking', default=0)
parser.add_option('-a', '--arichtrk', dest='arichtrk', default=0)
parser.add_option('-g', '--gdl', dest='gdl', default=0)
(options, args) = parser.parse_args()


# set specific database tag
# b2.conditions.override_globaltags(["tagname"])
# use local database
# b2.conditions.testing_payloads = ["localdb/database.txt"]

# create paths
main = b2.create_path()
store = b2.create_path()

# root input module
input_module = b2.register_module('SeqRootInput')
input_module.param('inputFileName', options.filename)
# input_module.param('entrySequences',['5100:5300']) # process only range of events
main.add_module(input_module)

# Histogram manager module
histo = b2.register_module('HistoManager')
histo.param('histoFileName', "DQMhistograms.root")  # File to save histograms
main.add_module(histo)

# build geometry if display option
if int(options.tracking):
    gearbox = b2.register_module('Gearbox')
    main.add_module(gearbox)
    geometry = b2.register_module('Geometry')
    geometry.param('useDB', 1)
    main.add_module(geometry)

# unpack raw data
unPacker = b2.register_module('ARICHUnpacker')
unPacker.param('debug', int(options.debug))
main.add_module(unPacker)

# create ARICHHits from ARICHDigits
arichHits = b2.register_module('ARICHFillHits')
# set bitmask for makin hits form digits
arichHits.param("bitMask", 0xFF)
main.add_module(arichHits)

if int(options.tracking):
    cdcunpacker = b2.register_module('CDCUnpacker')
    cdcunpacker.param('xmlMapFileName', "data/cdc/ch_map.dat")
    cdcunpacker.param('enablePrintOut', False)
    main.add_module(cdcunpacker)
    add_cosmics_reconstruction(main, 'CDC', False)

if int(options.gdl):
    trggdlUnpacker = b2.register_module("TRGGDLUnpacker")
    main.add_module(trggdlUnpacker)
    trggdlsummary = b2.register_module('TRGGDLSummary')
    main.add_module(trggdlsummary)

# create simple DQM histograms
arichHists = b2.register_module('ARICHDQM')
main.add_module(arichHists)

# store the dataobjects
if(options.output != ''):
    output = b2.register_module('RootOutput')
    output.param('outputFileName', options.output)
    branches = ['ARICHDigits', 'ARICHHits', 'ARICHInfo']
    if int(options.tracking):
        branches.extend(['Tracks', 'TrackFitResults', 'RecoTracks', 'RecoHitInformations', 'ExtHits'])
    if int(options.gdl):
        branches.append('TRGSummary')
    output.param('branchNames', branches)
    if int(options.arichtrk):
        store.add_module(output)
    else:
        main.add_module(output)

# show progress
progress = b2.register_module('Progress')
main.add_module(progress)

if int(options.arichtrk):
    arichHists.if_value('==1', store)

# process
b2.process(main)

# print stats
print(b2.statistics)
