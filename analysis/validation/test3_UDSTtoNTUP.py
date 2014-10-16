#!/usr/bin/env python
# -*- coding: utf-8 -*-

## This is a dummy script, ready to test UDST reading when ParticleLists are persistified

"""
<header>
  <input>MDSTtoUDST.udst.root</input>
  <contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

import sys

rootFileName = '../UDSTtoNTUP'
nOfEvents = 10
logFileName = rootFileName + '.log'

sys.stdout = open(logFileName, 'w')

import os
from basf2 import *

main = create_path()

# --------------------------------------------------------------
# set_log_level(LogLevel.DEBUG)
input = register_module('RootInput')
input.param('inputFileName', '../MDSTtoUDST.udst.root')
main.add_module(input)

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')
gearbox = register_module('Gearbox')
main.add_module(progress)
main.add_module(gearbox)

# ----------------------------------------------------------------
# Add the geometry for Rave
geometry = register_module('Geometry')
main.add_module(geometry)

# ---------------------------------------------------------------
# Here the ParticleLists will be extracted from the UDST to dump to ntuple

# ---------------------------------------------------------------
# Produce a microdst with Particles and MCParticles only (event meta data is included by default)
process(main)

# Print call statistics
print statistics
