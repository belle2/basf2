#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

rootFileName = '../MDSTtoUDST'
nOfEvents = -1
logFileName = rootFileName + '.log'
sys.stdout = open(logFileName, 'w')

import os
from basf2 import *
from modularAnalysis import *

main = create_path()

# --------------------------------------------------------------
# set_log_level(LogLevel.ERROR)
input = register_module('RootInput')
input.param('inputFileName', '../DSTtoMDST.mdst.root')
main.add_module(input)

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')
gearbox = register_module('Gearbox')
main.add_module(progress)
main.add_module(gearbox)

# ----------------------------------------------------------------
# Add the geometry, necessary for Rave
geometry = register_module('Geometry')
main.add_module(geometry)

# ---------------------------------------------------------------
# Run analysis tools to create Final State Particles (further tests to come)
loadReconstructedParticles(main)

# ---------------------------------------------------------------
# Produce a microdst with reconstructed Particles
outputUdst('../MDSTtoUDST.udst.root', main)

# Go
process(main)

# Print call statistics
print statistics
