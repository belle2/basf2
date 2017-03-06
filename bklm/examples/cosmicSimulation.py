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
from reconstruction import add_mdst_output

# Set the global log level
set_log_level(LogLevel.INFO)

# Set random seed
set_random_seed(888)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# Register the geometry module
geometry = register_module('Geometry')
geometry.param('components', ['BKLM'])

# Register the CRY module
cry = register_module('CRYInput')

# cosmic data input
cry.param('CosmicDataDir', Belle2.FileSystem.findFile('data/generators/modules/cryinput/'))

# user input file
cry.param('SetupFile', Belle2.FileSystem.findFile('generators/examples/cry.setup'))

# acceptance half-lengths - at least one particle has to enter that box to use that event
cry.param('acceptLength', 6.0)
cry.param('acceptWidth', 6.0)
cry.param('acceptHeight', 6.0)
cry.param('maxTrials', 100000)

# keep half-lengths - all particles that do not enter the box are removed (keep box >= accept box)
cry.param('keepLength', 6.0)
cry.param('keepWidth', 6.0)
cry.param('keepHeight', 6.0)

# minimal kinetic energy - all particles below that energy are ignored
cry.param('kineticEnergyThreshold', 0.01)

# Register the Progress module and the Python histogram module
progress = register_module('Progress')

# output
output = register_module('RootOutput')
output.param('outputFileName', './cry_bklm_mdst.root')


# Create the main path and add the modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)

# Reset the top volume: must be larger than the generated surface and higher than the detector
# It is the users responsibility to ensure a full angular coverage
main.add_module("Gearbox", override=[
    ("/Global/length", "50.", "m"),
    ("/Global/width", "50.", "m"),
    ("/Global/height", "50.", "m"),
])

# Full Geant4 simulation
g4sim = register_module('FullSim')

# BKLM digi
bklmdigi = register_module('BKLMDigitizer')
# BKLM reco
bklmreco = register_module('BKLMReconstructor')

main.add_module(geometry)
main.add_module(cry)
# uncomment the following line if you want event by event info
# main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)
main.add_module(g4sim)
main.add_module(bklmdigi)
main.add_module(bklmreco)

main.add_module(output)

# mdst output the second argument is MC
# add_mdst_output(main, True, 'cry_bklm_mdst.root')

# generate events
process(main)

# show call statistics
print(statistics)
