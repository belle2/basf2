#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.INFO)

packer = b2.register_module('DummyDataSource')
packer.param('NodeID', 4001)  # COPPER node ID (stored in RawHeader)
packer.param('UseShmFlag', 0)
packer.param('NodeName', "4001")

# Histo Module
histo = b2.register_module('DqmHistoManager')
histo.param('HostName', 'localhost')
histo.param('Port', 9991)
histo.param('DumpInterval', 1000)

# Monitor module
monitor = b2.register_module('MonitorDataCOPPER')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(packer)
main.add_module(histo)
main.add_module(monitor)

# Process all events
b2.process(main)
