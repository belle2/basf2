#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# VXDTF2 Example Scripts - Step 2 - Sector Map Training
#
# Performs the Sector Map Training based on the afore prepared training
# sample. This training is performed in the initialize phase, thus
# this script does not run over any actual events.
#
# The training sample must be provided via the extra commandline
# argument '-- --train_sample <f1> <f2> ...' or '-- -i <f1> <f2> ...'
# where f1, f2 and so on are different prepared training samples.
# The -- is requiered to escape from basf2 options and add additional
# commandline arguments specifically for this script.
# The file name under which the trained SecMap will be stored can be
# specified via the argument --secmap
# The pruning of the X least used relations can be specified via
# the argument --threshold X
#
# Usage: basf2 trainSecMap.py -- --train_sample traindata.root --secmap trainedSecMap.root --threshold 70
#
#
# Contributors: Jonas Wagner, Felix Metzner
#####################################################################

import basf2 as b2
import sys
import argparse
import os


# ---------------------------------------------------------------------------------------
# Argument parser for input of training sample file via comandline option.
arg_parser = argparse.ArgumentParser(description='Sector Map Training:\
                                     Trains and stores SecMap from provided data sample.\n\
                                     Usage: basf2 trainSecMap.py -- --train_sample traindata.root --secmap trainedSecMap.root')

arg_parser.add_argument('--train_sample', '-i', type=str, nargs='*',
                        help='List of prepared training data file names which will be used for the training of the SecMap')
arg_parser.add_argument('--secmap', '-s', type=str,
                        help='Inclusion of the root file containing the trained SecMap for the application of the VXDTF2.')
arg_parser.add_argument(
    '--threshold',
    '-t',
    type=int,
    default=70,
    help='Relative threshold (in %) used to prune the sector maps. Will remove X % of the least used subgraphs.')

arguments = arg_parser.parse_args(sys.argv[1:])
train_data = arguments.train_sample
secmap_name = arguments.secmap
relThreshold = arguments.threshold
assert len(train_data) > 0, 'No data sample for training provided!'

# ---------------------------------------------------------------------------------------
# Logging and Debug Level
b2.set_log_level(b2.LogLevel.ERROR)
b2.log_to_file('logVXDTF2Training.log', append=False)


# ---------------------------------------------------------------------------------------
path = b2.create_path()

# Event Info Setter which is requiered as the SecMap Training is performed in the Initilize
# Phase. Thus, this script does not run over any events at all, but over the data available
# in the prepared training sample root file.
eventinfosetter = b2.register_module('EventInfoSetter')
# default phase3 geometry:
exp_number = 0
# if environment variable is set then phase2 (aka Beast2) geometry will be taken
if os.environ.get('USE_BEAST2_GEOMETRY'):
    exp_number = 1002
eventinfosetter.param("expList", [exp_number])
path.add_module(eventinfosetter)

# puts the geometry and gearbox in the path
gearbox = b2.register_module('Gearbox')
path.add_module(gearbox)
# the geometry is loaded from the DB by default now! The correct geometry
# should be pickked according to exp number
geometry = b2.register_module('Geometry')
path.add_module(geometry)


# SecMapBootStrap Module is requiered, as it holds all  the sector maps and
# for storing the trained SecMap.
secMapBootStrap = b2.register_module('SectorMapBootstrap')
secMapBootStrap.param('ReadSectorMap', False)
secMapBootStrap.param('WriteSectorMap', True)
if secmap_name:
    secMapBootStrap.param('SectorMapsOutputFile', secmap_name)
elif os.environ.get('USE_BEAST2_GEOMETRY'):
    secMapBootStrap.param('SectorMapsOutputFile', 'SectorMaps_Beast2.root')
path.add_module(secMapBootStrap)

# Perform SecMap Training on provided data sample
merger = b2.register_module('RawSecMapMerger')
merger.param('rootFileNames', train_data)
merger.param('threshold', relThreshold)
path.add_module(merger)

b2.print_path(path)
b2.process(path)
print(b2.statistics)
