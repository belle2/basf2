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

main.add_module('EventInfoSetter',
                evtNumList=[100],  # we want to process 100 events
                runList=[1],  # from run number 1
                expList=[0],  # and experiment number 0
                )

# Register the CRY module
cry = main.add_module('CRYInput')
# acceptance half-lengths - at least one particle has to enter that box
# (without simulation, based on generator kinematics only!) to use that
# event. All values in cm
# 1 Parameter: sphere with the given radius around the origin
# 2 parameters: xy radius and z half-length of a cylinder along the z-axis
# 3 parameters: x, y and z half-lengths of a box at the origin.
cry.param('acceptance', [125, 125])
# How many trials to generate a cosmic per event?
cry.param('maxTrials', 100000)
# minimal kinetic energy in GeV - all particles below that energy are ignored
cry.param('kineticEnergyThreshold', 0.01)

main.add_module('Progress')
main.add_module('RootOutput', outputFileName='./cry-outfile-test.root')
# uncomment the following line if you want event by event info
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
