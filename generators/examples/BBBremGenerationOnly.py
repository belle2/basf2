#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

########################################################
# 10000 radiative Bhabha events are generated using the BBBrem
# generator
#
# Example steering file
########################################################

import basf2

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=10000)

# Register the BBBremInput module
bbbrem = basf2.register_module('BBBremInput')

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
progress = basf2.register_module('Progress')

# output
output = basf2.register_module('RootOutput')
output.param('outputFileName', './bbbremgen.root')

# Create the main path and add the modules

main.add_module(progress)
main.add_module(bbbrem)

main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

main.add_module(output)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
