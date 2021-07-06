#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
######################################################

import basf2 as b2

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
input = b2.register_module('RootInput')
# input = register_module('SeqRootInput')
# input.param('inputFileName', '/home/usr/yamadas/e0000r000634.binary') # only for SeqRootInput

# output
output = b2.register_module('PrintData')

# dump
dump = b2.register_module('RootOutput')
dump.param('outputFileName', 'temp.root')

# Histogram
# hist = register_module('HistoExercise1')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(output)
# main.add_module(hist)
# main.add_module(dump)

# Process all events
b2.process(main)
