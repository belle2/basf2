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
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

# input
input = b2.register_module('SeqRootInput')
# input.param('inputFileName', '/x02/data/e0000r000554.sroot')

# output
output = b2.register_module('MakeDumHSLBData')

prog = b2.register_module('Progress')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(output)
main.add_module(prog)

# Process all events
b2.process(main)
