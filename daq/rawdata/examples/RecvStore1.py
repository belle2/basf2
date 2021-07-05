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
#
# Example steering file - 2011 Belle II Collaboration
######################################################

import basf2 as b2

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# Modules
receiver = b2.register_module('DeSerializer')
dump = b2.register_module('PrintCollections')
# perf = register_module('DAQPerf')
output = b2.register_module('RootOutput')

# RxSocket
receiver.param('Port', [99999])

# Perf
# perf.param('Cycle', 100000)

# Output
output.param('outputFileName', 'rawcopperM.root')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(receiver)
# main.add_module(dump)
main.add_module(output)
# main.add_module(perf)

# Process all events
b2.process(main)
