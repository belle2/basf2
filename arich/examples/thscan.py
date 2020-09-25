#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# RecvStream0.py : DAQ component running on a readout PC.
# This component receive data from COPPER and send data to eb0 process.
#
######################################################

import basf2 as b2
import os
from optparse import OptionParser

from basf2 import conditions
conditions.override_globaltags()
conditions.append_globaltag('online')
conditions.append_globaltag('ARICH_phase3_test')


# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.INFO)
# parameters
parser = OptionParser()
parser.add_option('-i', '--inputpath', dest='path', default='')
(options, args) = parser.parse_args()

input = b2.register_module('SeqRootInput')
file_list = [options.path + f for f in os.listdir(options.path) if f.endswith('.sroot')]

input.param('inputFileNames', file_list)

histo = b2.register_module('HistoManager')

cal = b2.register_module('ARICHRateCal')
cal.param("nrun", 100)
cal.param("nevents", 1000)
cal.param("dth", 0.0096)  # can be ignored when internal = True
cal.param("th0", -0.48)  # can be ignored when internal = True
cal.param("internal", True)

unpack = b2.register_module('ARICHUnpacker')
# unpack.param('RawUnpackerMode', 1)
# unpack.param('DisableUnpackerMode', 1)

convert = b2.register_module('Convert2RawDet')
output = b2.register_module('RootOutput')
progress = b2.register_module('Progress')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(histo)
main.add_module(convert)
main.add_module(unpack)
main.add_module(cal)
main.add_module(progress)

# Process all events
b2.process(main)
