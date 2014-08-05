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
import sys
argvs = sys.argv

if len(argvs) < 2:
    print 'Usage : DummyDataPacker.py <# of events>'
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
max_event = int(argvs[1])
packer = register_module('DummyDataPacker')
packer.param('MaxEventNum', max_event)
# root = register_module('RootOutput')
# root.param('outputFileName', argvs[2])
dump = register_module('Ds2RawFile')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(packer)
# main.add_module(root)
main.add_module(dump)

# Process all events
process(main)
