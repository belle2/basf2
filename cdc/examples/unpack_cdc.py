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
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))

# Input (ROOT file).
input = register_module('RootInput')
# Input (Seq. ROOT file).
# input = register_module('SeqRootInput')
unpacker = register_module('CDCUnpacker')
output = register_module('RootOutput')
output.param('outputFileName', 'UnpackerOutput.root')
output.param('branchNames', ['CDCHits', 'CDCRawHits'])

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)
main.add_module(output)

# Process all events
print_path(main)
process(main)

print(statistics)
