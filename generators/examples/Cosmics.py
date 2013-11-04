#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# This steering file which shows all usage options for the cosmics module
# in the generators package. The generated particles from the cosmic generator
# are then fed through a full Geant4 simulation and the output is stored in
# a root file.
#
# The different options for the particle gun are explained below.
# Uncomment/comment different lines to get the wanted settings
#
# Example steering file - 2011 Belle II Collaboration
##############################################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# to run the framework the used modules need to be registered
cosmics = register_module('Cosmics')
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
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# Save output of simulation
output = register_module('RootOutput')

# Setting the option for all modules: want to process 100 MC events
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# Set output filename
output.param('outputFileName', 'CosmicsOutput.root')

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(cosmics)
main.add_module(simulation)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
