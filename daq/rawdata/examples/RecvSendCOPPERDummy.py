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

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Dummy data generator
print argvs[1]
print argvs[2]
print argvs[3]
max_event = int(argvs[3])
nodeid = int(argvs[2])
packer = register_module('DummyDataSource')
# packer.param('MaxEventNum', max_event)
packer.param('NodeID', nodeid)

# File output
dump = register_module('RootOutput')
dump.param('outputFileName', 'root_output.root')

# Sender
sender = register_module('Serializer')
sender.param('DestPort', 33000)
# sender.param('LocalHostName', 'cpr006')
sender.param('LocalHostName', argvs[1])

# Create main path
main = create_path()

# Add modules to main path
main.add_module(packer)
# if use_shm_flag != 0:
#    main.add_module(histo)
#    main.add_module(monitor)
# main.add_module(dump)
main.add_module(sender)

# Process all events
process(main)
