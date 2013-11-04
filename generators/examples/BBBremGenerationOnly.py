#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# 10000 radiative Bhabha events are generated using the BBBrem
# generator
#
# Example steering file
########################################################

from basf2 import *

## Set the global log level
set_log_level(LogLevel.INFO)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])  # we want to process 10 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

## Register the BBBremInput module
bbbrem = register_module('BBBremInput')

# Set the mode for the boost of the generated particles 0 = no boost 1 = BELLE
# II 2 = BELLE
bbbrem.param('BoostMode', 1)

# Set the fraction of the minimum photon energy.
# bbbrem.param('MinPhotonEnergyFraction', 0.000001)

# Produce unweighted or weighted events.
# bbbrem.param('Unweighted', True)

# Set the max weight (only for Unweighted=True).
# bbbrem.param('MaxWeight', 2000.0)

# Set the logging level for the BHWide module to INFO in order to see the total
# cross section
# bbbrem.set_log_level(LogLevel.INFO)

# geometry parameter database
gearbox = register_module('Gearbox')

## Register the Progress module and the Python histogram module
progress = register_module('Progress')

# printmc = register_module("PrintMCParticles")
# printmc.param('collectionName', 'MCParticles')
# main.add_module(printmc)

# output
output = register_module('RootOutput')
output.param('outputFileName', './bbbremgen.root')

## Create the main path and add the modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(bbbrem)
# main.add_module(printmc)
main.add_module(output)

# generate events
process(main)

# show call statistics
print statistics
