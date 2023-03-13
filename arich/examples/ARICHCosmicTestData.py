#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Script for ARICH cosmics data simple analysis
# reads arich raw data, stores arichHits and arichDigits into the output root file,
# makes and displays DQM histograms, runs the event display
#
# run as: "basf2 ARICHCosmicTestData.py -- -f path2datafile.sroot"
# add "-s 1" for event display
#
# two parameters (MinHits and MaxHits) are available for DQM module (check below)
# only events with number of hits within the range are included in DQM histograms
# and shown in the event display (set to 5 and 40, but change accoring your need)

import basf2 as b2
import os
from optparse import OptionParser
home = os.environ['BELLE2_LOCAL_DIR']


b2.conditions.override_globaltags()
b2.conditions.append_globaltag("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000")

# parameters
parser = OptionParser()
parser.add_option(
    '-f',
    '--file',
    dest='filename',
    default='/ghi/fs01/belle2/bdata/users/tkonno/cosmic/cosmic.0002.00951.HLT3.f00000.root')
parser.add_option('-o', '--output', dest='output', default='ARICHHits.root')
parser.add_option('-d', '--debug', dest='debug', default=0)
parser.add_option('-s', '--display', dest='display', default=0)
(options, args) = parser.parse_args()

# create paths
main = b2.create_path()
displ = b2.create_path()


# root input module
input_module = b2.register_module('RootInput')
input_module.param('inputFileName', options.filename)
# input_module.param('entrySequences',['5100:5300']) # select subrange of events
main.add_module(input_module)

# Histogram manager module
histo = b2.register_module('HistoManager')
histo.param('histoFileName', "histograms.root")  # File to save histograms
main.add_module(histo)


# build geometry if display option
if int(options.display):
    main.add_module('Gearbox')
    geometry = b2.register_module('Geometry')
    geometry.param('components', ['ARICH'])
    main.add_module(geometry)

# unpack raw data
unPacker = b2.register_module('ARICHUnpacker')
unPacker.param('debug', int(options.debug))
main.add_module(unPacker)

# create ARICHHits from ARICHDigits
main.add_module('ARICHFillHits')

# create simple DQM histograms
arichHists = b2.register_module('ARICHDQM')
arichHists.param('MaxHits', 40)
arichHists.param('MinHits', 5)
main.add_module(arichHists)

# add display module if display option
if int(options.display):
    display = b2.register_module('Display')
    # show arich hits
    display.param('showARICHHits', True)
    # show full geometry
    display.param('fullGeometry', True)
    displ.add_module(display)

# store dataobjects
output = b2.register_module('RootOutput')
output.param('outputFileName', options.output)
branches = ['ARICHDigits', 'ARICHHits']
output.param('branchNames', branches)
main.add_module(output)

# show progress
main.add_module('Progress')

arichHists.if_value('==1', displ)

# process
b2.process(main)

# print stats
print(b2.statistics)

# plot DQM histograms
if not int(options.display):
    com = 'root -l histograms.root ' + home + '/arich/utility/scripts/plotDQM.C'
    os.system(com)
