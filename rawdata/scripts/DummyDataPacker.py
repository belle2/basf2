#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys
argvs = sys.argv

if len(argvs) < 3:
    print('Usage : DummyDataPacker.py <# of events> <output filename>')
    sys.exit()

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

# Reader
max_event = int(argvs[1])
packer = b2.register_module('DummyDataPacker')
packer.param('MaxEventNum', max_event)

# File output
# dump = register_module('SeqRootOutput')
dump = b2.register_module('RootOutput')
dump.param('outputFileName', argvs[2])

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(packer)
main.add_module(dump)

# Process all events
b2.process(main)
