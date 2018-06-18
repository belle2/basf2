#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This steering unpacking the CDC rawdata.
#
######################################################

from basf2 import *
from ROOT import Belle2

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

# Set Database
reset_database()
use_database_chain()
use_central_database("Calibration_Offline_Development", LogLevel.INFO)

# Input file
# Get type of input file to decide, which input module we want to use
input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = register_module('SeqRootInput')
else:
    root_input = register_module('RootInput')

unpacker = register_module('CDCUnpacker')
output = register_module('RootOutput')
output.param('outputFileName', 'UnpackerOutput.root')
output.param('branchNames', ['CDCHits', 'CDCRawHits'])

# Create main path
main = create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(unpacker)
main.add_module(output)

# Process all events
print_path(main)
process(main)

print(statistics)
