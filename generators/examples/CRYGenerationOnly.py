#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 100 cosmic events are generated using the
# CRY generator
#
# Example steering file
########################################################

import basf2
from ROOT import Belle2

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

# Set random seed
basf2.set_random_seed(888)

main = basf2.create_path()

eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [0])  # and experiment number 0

# Register the geometry module
geometry = basf2.register_module('Geometry')

# Register the CRY module
cry = basf2.register_module('CRYInput')

# cosmic data input
cry.param('CosmicDataDir', Belle2.FileSystem.findFile('data/generators/modules/cryinput/'))

# user input file
cry.param('SetupFile', 'cry.setup')

# acceptance half-lengths - at least one particle has to enter that box
# (without simulation, based on generaor kinematics only!) to use that
# event
cry.param('acceptLength', 6.0)
cry.param('acceptWidth', 6.0)
cry.param('acceptHeight', 6.0)
cry.param('maxTrials', 100000)

# keep half-lengths - all particles that do not enter the box are removed
# (keep box >= accept box)  (without simulation, based on generaor
# kinematics only!)
cry.param('keepLength', 6.0)
cry.param('keepWidth', 6.0)
cry.param('keepHeight', 6.0)

# minimal kinetic energy - all particles below that energy are ignored
cry.param('kineticEnergyThreshold', 0.01)

# Register the Progress module and the Python histogram module
progress = basf2.register_module('Progress')

# output
output = basf2.register_module('RootOutput')
output.param('outputFileName', './cry-outfile-test.root')

main.add_module(eventinfosetter)
main.add_module(progress)

# Reset the top volume: must be larger than the generated surface and higher than the detector
# It is the users responsibility to ensure a full angular coverage
main.add_module("Gearbox", override=[
    ("/Global/length", "50.", "m"),
    ("/Global/width", "50.", "m"),
    ("/Global/height", "50.", "m"),
])
main.add_module(geometry)
main.add_module(cry)
main.add_module(output)
# uncomment the following line if you want event by event info
# main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
