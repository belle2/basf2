#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This steering unpacking the CDC rawdata.
#
######################################################

from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)


# Input (ROOT file).
input = register_module('RootInput')
# Input (Seq. ROOT file).
# input = register_module('SeqRootInput')
# output
unpacker = register_module('CDCUnpacker')
# FE channel <-> CDC cell ID map.
unpacker.param('xmlMapFileName', 'ch_map.dat')
# Enable/Disable print out the ADC/TDC data to the terminal.
unpacker.param('enablePrintOut', True)
# Set/Unset the relation between RawCDC and CDCHit.
unpacker.param('setRelationRaw2Hit', False)

output = register_module('RootOutput')
output.param('outputFileName', 'UnpackerOutput.root')


# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)
main.add_module(output)
# Process all events
process(main)

print(statistics)
