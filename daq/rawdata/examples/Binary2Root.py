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

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
input = b2.register_module('DeSerializerFILE')
# input.param('inputFileName', 'cdc_10kHz_newFEEformat_131206.dat')
# input.param('inputFileName', 'RUN20131206170859.dat')
input.param('inputFileName',
            '/home/usr/yamadas/work/svddummy/RUN19880101061243_wAPVheaderdetection.dat'
            )
# input.param('inputFileName', '/home/usr/yamadas/work/svddummy/RUN19880101060100_woAPVheaderdetection.dat')
# input.param('inputFileName', '/home/usr/yamadas/work/svddummy/RUN19880101143745_transparent.dat')

input.param('inputRepetitionTimes', 0)

# output
# output = register_module('PrintDataTemplate')
# output = register_module('Root2Binary')

# dump
dump = b2.register_module('RootOutput')
dump.param('outputFileName', 'temp.root')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
# main.add_module(output)
main.add_module(dump)

# Process all events
b2.process(main)
