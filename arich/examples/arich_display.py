# !/usr/bin/env python3
# -*- coding: utf-8 -*-

# Script for running arich event display, DQM module, and arich reconstruction on
# processed data (containing arichHits)
# To process raw data first use arich/examples/arich_process_raw.py
#
# run as: "basf2 arich/examples/arich_display.py -- -f path2processedDatafile.sroot"
# add "-s 1" for event display, otherwise only DQM histograms are produced
# add "-a 1" to select only events that have extrapolated track in the arich
# add "-r 1" to add the arich reconstruction (for cherenkov angle distribution)
#
# two parameters (MinHits and MaxHits) are available for DQM module (check below)
# only events with number of hits within the range are included in DQM histograms
# and shown in the event display (set to 5 and 40, but change accoring your need)
#
# Author: Luka Santelj

from basf2 import *
import os
from optparse import OptionParser
home = os.environ['BELLE2_LOCAL_DIR']


# reset_database()
# use_central_database("development")
use_central_database("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000")

# parameters
parser = OptionParser()
parser.add_option('-f', '--file', dest='filename', default='ARICHHits.root')
parser.add_option('-s', '--display', dest='display', default=0)
parser.add_option('-a', '--arichtrk', dest='arichtrk', default=0)
parser.add_option('-r', '--recon', dest='recon', default=0)
(options, args) = parser.parse_args()

# create paths
main = create_path()
displ = create_path()

# root input module
input_module = register_module('RootInput')
input_module.param('inputFileName', options.filename)
main.add_module(input_module)

# Histogram manager module
histo = register_module('HistoManager')
histo.param('histoFileName', "histograms.root")  # File to save histograms
main.add_module(histo)

# build geometry if display option
if int(options.display):
    gearbox = register_module('Gearbox')
    main.add_module(gearbox)
    geometry = register_module('Geometry')
    geometry.param('components', ['ARICH', 'MagneticField'])
    main.add_module(geometry)

if int(options.recon):
    arichreco = register_module('ARICHReconstructor')
    arichreco.param('storePhotons', 1)
    main.add_module(arichreco)
    arichNtuple = register_module('ARICHNtuple')
    arichNtuple.param('outputFile', 'arich_recon_ntuple.root')
    main.add_module(arichNtuple)


# create simple DQM histograms
arichHists = register_module('ARICHDQM')
arichHists.param('ArichEvents', bool(options.arichtrk))
# set hit range - include only events with hits in this range (also for event display!)
arichHists.param('MaxHits', 100)
arichHists.param('MinHits', 0)
main.add_module(arichHists)

# add display module if display option
if int(options.display):
    display = register_module('Display')
    # show arich hits
    display.param('showARICHHits', True)
    # show reconstruced tracks
    display.param('showRecoTracks', True)
    # show full geometry
    display.param('fullGeometry', True)
    displ.add_module(display)


# show progress
progress = register_module('Progress')
main.add_module(progress)

arichHists.if_value('==1', displ)

# process
process(main)

# print stats
print(statistics)

# plot DQM histograms
if not int(options.display):
    com = 'root -l histograms.root ' + home + '/arich/utility/scripts/plotDQM.C'
    os.system(com)
