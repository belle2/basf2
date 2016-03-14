#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 10000 radiative Bhabha events are generated using the BBBrem
# generator
#
# Example steering file
########################################################

from basf2 import *
from beamparameters import add_beamparameters

# Set the global log level
set_log_level(LogLevel.INFO)

main = create_path()

main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=10000)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
beamparameters.param("smearVertex", True)
beamparameters.param("generateCMS", False)
# print_params(beamparameters)

# Register the BBBremInput module
bbbrem = register_module('BBBremInput')

# Set the fraction of the minimum photon energy.
# bbbrem.param('MinPhotonEnergyFraction', 0.000001)

# Produce unweighted or weighted events.
bbbrem.param('Unweighted', True)

# Set the max weight (only for Unweighted=True).
bbbrem.param('MaxWeight', 5.0e6)

# Set the mode for bunch density correction (none=0, hard=1 (default), soft=2).
bbbrem.param('DensityCorrectionMode', 1)

# Set the Density correction parameter tc.
bbbrem.param('DensityCorrectionParameter', 1.68e-17)

# Set the logging level for the BBBREM module to INFO
# bbbrem.set_log_level(LogLevel.INFO)

# Register the Progress module and the Python histogram module
progress = register_module('Progress')

# output
output = register_module('RootOutput')
output.param('outputFileName', './bbbremgen.root')

# Create the main path and add the modules

main.add_module(progress)
main.add_module(bbbrem)

main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

main.add_module(output)

# generate events
process(main)

# show call statistics
print(statistics)
