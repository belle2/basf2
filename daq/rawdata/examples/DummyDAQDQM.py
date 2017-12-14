#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

packer = register_module('DummyDataSource')
packer.param('NodeID', 4001)  # COPPER node ID (stored in RawHeader)
packer.param('UseShmFlag', 0)
packer.param('NodeName', "4001")

# Histo Module
histo = register_module('DqmHistoManager')
histo.param('HostName', 'localhost')
histo.param('Port', 9991)
histo.param('DumpInterval', 1000)

# Monitor module
monitor = register_module('MonitorDataCOPPER')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(packer)
main.add_module(histo)
main.add_module(monitor)

# Process all events
process(main)
