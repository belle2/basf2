#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argvs = sys.argv

if len(argvs) < 3:
    print 'Usage : DummyDataPacker.py <# of events> <output filename>'
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Reader
max_event = int(argvs[1])
packer = register_module('DummyDataPacker')
packer.param('MaxEventNum', max_event)


# File output
# dump = register_module('SeqRootOutput')
dump = register_module('RootOutput')
dump.param('outputFileName', argvs[2])

# Create main path
main = create_path()

# Add modules to main path
main.add_module(packer)
main.add_module(dump)

# Process all events
process(main)
