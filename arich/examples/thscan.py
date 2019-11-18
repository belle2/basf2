#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# RecvStream0.py : DAQ comoponent running on a readout PC.
# This component receive data from COPPER and send data to eb0 process.
#
######################################################

from basf2 import *
import os
from optparse import OptionParser

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)
reset_database()
use_database_chain()

# set DB tag with correct merger numbers, etc. (phase3 start)
use_central_database('online')
use_central_database('ARICH_phase3_test')


# parameters
parser = OptionParser()
parser.add_option('-i', '--inputpath', dest='path', default='')
(options, args) = parser.parse_args()

input = register_module('SeqRootInput')
file_list = [options.path + f for f in os.listdir(options.path) if f.endswith('.sroot')]

input.param('inputFileNames', file_list)

histo = register_module('HistoManager')

cal = register_module('ARICHRateCal')
cal.param("nrun", 100)
cal.param("nevents", 1000)
cal.param("dth", 0.01)  # can be ignored when internal = True
cal.param("th0", -0.5)  # can be ignored when internal = True
cal.param("internal", True)

unpack = register_module('ARICHUnpacker')
unpack.param('RawUnpackerMode', 1)
unpack.param('DisableUnpackerMode', 1)

convert = register_module('Convert2RawDet')
output = register_module('RootOutput')
progress = register_module('Progress')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(histo)
main.add_module(convert)
main.add_module(unpack)
main.add_module(cal)
main.add_module(progress)

# Process all events
process(main)
