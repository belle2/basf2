#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2

# suppress messages and warnings during processing:
basf2.set_log_level(basf2.LogLevel.ERROR)

# to run the framework the used modules need to be registered
cosmics = basf2.register_module('Cosmics')
cosmics.param('level', 1)
cosmics.param('ipRequirement', 0)
# ipdr and ipdz are only relevant for level = 1 and ipRequirement = 1
# cosmics.param('ipdr', 3.)
# cosmics.param('ipdz', 3.)
# cosmics.param('ptmin', 0.7)

# ============================================================================
# Setting the random seed for particle generation: this number can be any int,
# preferably large a value of 0 will use a different random seed each time
# default is 3452346.
# set_random_seed(1028307)

# ============================================================================
# Now lets create the necessary modules to perform a simulation
#
# Create Event information
eventinfosetter = basf2.register_module('EventInfoSetter')
# Show progress of processing
progress = basf2.register_module('Progress')

# Save output of generation
output = basf2.register_module('RootOutput')

# Setting the option for all modules: want to process 100 MC events
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# Set output filename
output.param('outputFileName', 'CosmicsOutput.root')

# ============================================================================
# Do the simulation

main = basf2.create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(cosmics)
main.add_module(output)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
