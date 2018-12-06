#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#
# Usage: basf2 trainSecMap.py -- --train_sample traindata.root --secmap trainedSecMap.root
#
#
# Contributors: Jonas Wagner, Felix Metzner
#####################################################################


from basf2 import *
import argparse
from testbeam.utils import *


# ---------------------------------------------------------------------------------------
# Argument parser for input of training sample file via comandline option.
arg_parser = argparse.ArgumentParser(description='Sector Map Training:\
                                     Trains and stores SecMap from provided data sample.\n\
                                     Usage: basf2 trainSecMap.py -- --train_sample traindata.root --secmap trainedSecMap.root')

arg_parser.add_argument('--train_sample', '-i', type=str, action='append',
                        help='List of prepared training data file names which will be used for the training of the SecMap')
# arg_parser.add_argument('--secmap', '-s', type=str,
#                        help='Inclusion of the root file containing the trained SecMap for the application of the VXDTF2.')

arguments = arg_parser.parse_args(sys.argv[1:])
train_data = arguments.train_sample
assert len(train_data) > 0, 'No data sample for training provided!'
assert len(train_data) <= 1, 'Only one input file supported!'
# secmap_name = arguments.secmap
# use the same name base as the input data (first in the list) to have it reproducable
secmap_name = "SecMap_" + train_data[0]

# ---------------------------------------------------------------------------------------
# Logging and Debug Level
set_log_level(LogLevel.ERROR)
log_to_file('logVXDTF2Training.log', append=False)


# ---------------------------------------------------------------------------------------
path = create_path()

# Event Info Setter which is requiered as the SecMap Training is performed in the Initilize
# Phase. Thus, this script does not run over any events at all, but over the data available
# in the prepared training sample root file.
eventinfosetter = register_module('EventInfoSetter')
path.add_module(eventinfosetter)


# retrieve geometry and magnet from name of input file
# assumes that files (and names) are created by trainingPreparation.py
geometry_file = ""
if "afterMarch1st" in train_data[0]:
    geometry_file = "testbeam/vxd/2017_geometry_1.xml"
else:
    geometry_file = "testbeam/vxd/2017_geometry.xml"


magnetOn = False
if "MagnetOn" in train_data[0]:
    magnetOn = True
if "MagnetOff" in train_data[0]:
    magnetOn = False

print("using magnet on = " + str(magnetOn))
print("using geometry = " + geometry_file)

# removed the upstream scintilators, as they may produce curlers
add_geometry(path, magnet=magnetOn, geometry_xml=geometry_file,
             geometry_version=-666, excluded_components=['Scintilators'])

# SecMapBootStrap Module is requiered, as it loads the SecMap config and is responsible
# for storing the trained SecMap.
secMapBootStrap = register_module('SectorMapBootstrap')
secMapBootStrap.param('ReadSectorMap', False)
secMapBootStrap.param('WriteSectorMap', True)
if secmap_name:
    secMapBootStrap.param('SectorMapsOutputFile', secmap_name)
path.add_module(secMapBootStrap)

# Perform SecMap Training on provided data sample
merger = register_module('RawSecMapMerger')
merger.param('rootFileNames', train_data)
path.add_module(merger)


process(path)
print(statistics)
