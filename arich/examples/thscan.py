#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# RecvStream0.py : DAQ comoponent running on a readout PC.
# This component receive data from COPPER and send data to eb0 process.
#
######################################################

from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

# input = register_module('RootInput')
input = register_module('SeqRootInput')

histo = register_module('HistoManager')

cal = register_module('ARICHRateCal')
cal.param("nrun", 100)
cal.param("nevents", 1000)
cal.param("dth", 0.01)
cal.param("th0", -0.5)

unpack = register_module('ARICHRawUnpacker')

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
