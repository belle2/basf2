#!/usr/bin/env python3
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

import basf2 as b2
import sys
argvs = sys.argv

if len(argvs) < 3:
    print('Usage : DummyDataPacker.py <runno> <maxevents>')
    sys.exit()

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
# runno = int(argvs[1])
# max_event = int(argvs[2])
# packer = register_module('DummyDataPacker')
# packer.param('RunNumber', runno)
# packer.param('MaxEventNum', max_event)
# root = register_module('RootOutput')
# root.param('outputFileName', argvs[2])
# dump = register_module('Ds2RawFile')

root = b2.register_module('SeqRootInput')
# root.param('inputFileName', argvs[2])

output = b2.register_module('StorageSerializer')
output.param('OutputBufferName', 'OTEST')
output.param('OutputBufferSize', 10)
output.param('compressionLevel', 0)

# Create main path
main = b2.create_path()

# Add modules to main path
# main.add_module(packer)
main.add_module(root)
# main.add_module(dump)
main.add_module(output)

# Process all events
b2.process(main)
