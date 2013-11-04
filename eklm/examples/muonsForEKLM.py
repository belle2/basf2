#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file is based on
# simulation/examples/FullGeant4SimulationParticleGun.py
#
# Muons (and ani-muons) are simulated for EKLM testing
#
# 100 events for experiment and run number 1 are created.
#
# The following parameters are used:
#  Number of events:      100
#  Tracks per event:      2
#  Particles:             mu+ / mu -
#  Theta [default]:       17 to 150 degree
#  Phi [default]:         0 to 360 degree
#  Momentum:              50 MeV to 3 GeV
#
# Example steering file - 2011 Belle II Collaboration
########################################################

import os
import random
from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [20])

# Particle gun
particlegun = register_module('ParticleGun')
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [13, -13])
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1, 3])

particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [17, 150])

particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

# Geometry parameter loader
paramloader = register_module('Gearbox')

# Geometry builder
geobuilder = register_module('Geometry')
geobuilder.log_level = LogLevel.INFO

geobuilder.param('Components', ['EKLM'])

# Full Geant4 simulation
g4sim = register_module('FullSim')

# Root file output
output = register_module('RootOutput')
output.param('outputFileName', 'muForEKLM.root')

# MC printuots
mcprint = register_module('PrintMCParticles')

# ------------  EKLM-related modules

# EKLM digi
eklmdigi = register_module('EKLMDigitizer')
eklmdigi.log_level = LogLevel.INFO
# EKLM reco
eklmreco = register_module('EKLMReconstructor')
eklmreco.log_level = LogLevel.INFO

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)

main.add_module(eklmdigi)
main.add_module(eklmreco)

main.add_module(output)

# Process 100 events
process(main)

