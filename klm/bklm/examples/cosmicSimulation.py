#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 1000 cosmic events are generated using the
# CRY generator
#
# Example steering file
########################################################

from basf2 import *
from ROOT import Belle2
from generators import add_cosmics_generator

from reconstruction import add_mdst_output

# Set the global log level
from simulation import add_simulation

set_log_level(LogLevel.INFO)

# Set random seed
set_random_seed(888)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# Register the Progress module and the Python histogram module
progress = register_module('Progress')

# output
output = register_module('RootOutput')
output.param('outputFileName', './cry_bklm_mdst.root')


# Create the main path and add the modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)

add_cosmics_generator(main,
                      global_box_size=[50, 50, 50], accept_box=[6, 6, 6],
                      keep_box=[6, 6, 6],
                      setup_file='generators/examples/cry.setup')

add_simulation(main)

# uncomment the following line if you want event by event info
# main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

main.add_module(output)

# mdst output the second argument is MC
# add_mdst_output(main, True, 'cry_bklm_mdst.root')

# generate events
process(main)

# show call statistics
print(statistics)
