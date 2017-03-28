#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# Create main path
main = create_path()

# Modules
deserializer = register_module('DepfetPFDeserializer')
deserializer.param('InputBufferName', argv[1])
deserializer.param('InputBufferSize', int(argv[2]))
if len(argv) >= 6:
    deserializer.param('NodeName', argv[5])
    deserializer.param('NodeID', int(argv[6]))
    deserializer.param('UseShmFlag', int(argv[7]))

# Add modules to main path
main.add_module(deserializer)

# output = register_module('SeqRootOutput')
# output.param('compressionLevel', 1)
# main.add_module(output)

# Process all events
process(main)
