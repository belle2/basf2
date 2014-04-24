#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to run the framework
# for different experiment, run and event numbers.
#
# In the example below, basf2 will run on and display
# the following experiment, run and event numbers:
#
# Experiment 71, Run  3, 4 Events
# Experiment 71, Run  4, 6 Events
# Experiment 73, Run 10, 2 Events
# Experiment 73, Run 20, 5 Events
# Experiment 73, Run 30, 3 Events
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# Create main path
main = create_path()

# input
# input = register_module('SeqRootInput')
input = register_module('RootInput')
input.param('inputFileName', '/user/b2daq/rawroot/RawData_00000_000000.root')

# output
output = register_module('PrintData')

# Histo Module
# histo = register_module('HistoManager')
histo = register_module('DqmHistoManager')
histo.param('HostName', 'belle-rpc2')
histo.param('Port', 40010)

# Monitor module
monitor = register_module('MonitorDataCOPPER')

progress = register_module('Progress')

# Add modules to main path
main.add_module(input)
main.add_module(histo)
main.add_module(monitor)
main.add_module(progress)
# Process all events
process(main)
