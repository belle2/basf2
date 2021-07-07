#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
#
# This steering unpacking the CDC rawdata.
#
######################################################

import basf2 as b2
from ROOT import Belle2

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.INFO)

# Set Database
b2.reset_database()
b2.use_database_chain()
b2.use_central_database("Calibration_Offline_Development", b2.LogLevel.INFO)

# Input file
# Get type of input file to decide, which input module we want to use
input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = b2.register_module('SeqRootInput')
else:
    root_input = b2.register_module('RootInput')

unpacker = b2.register_module('CDCUnpacker')
output = b2.register_module('RootOutput')
output.param('outputFileName', 'UnpackerOutput.root')
output.param('branchNames', ['CDCHits', 'CDCRawHits'])

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(unpacker)
main.add_module(output)

# Process all events
b2.print_path(main)
b2.process(main)

print(b2.statistics)
