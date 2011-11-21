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
set_log_level(LogLevel.ERROR)

# EvtMetaGen - generate event meta data
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [2])
evtmetagen.param('RunList', [1])

# Particle gun
particlegun = register_module('PGunInput')
particlegun.param('nTracks', 1)
particlegun.param('PIDcodes', [13, -13])
particlegun.param('pPar1', 2)
particlegun.param('pPar2', 3)

# Geometry parameter loader
paramloader = register_module('Gearbox')

# Geometry builder
geobuilder = register_module('Geometry')
geobuilder.log_level = LogLevel.INFO

geobuilder.param('Components', ['EKLM'])

# Full Geant4 simulation
g4sim = register_module('FullSim')
g4sim.param('EnableVisualization', True)
g4sim.param('EnableVisualization', False)
g4sim.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
            '/vis/scene/add/axes 0 0 0 100 mm',
            '/vis/scene/add/trajectories smooth',
            '/vis/modeling/trajectories/create/drawByCharge'])

# Root file output
simpleoutput = register_module('SimpleOutput')
simpleoutput.param('outputFileName', 'muForEKLM.root')

# MC printuots
mcprint = register_module('PrintMCParticles')

# ------------  EKLM-related modules

# EKLM digi
eklmdigi = register_module('EKLMDigitization')
# EKLM reco
# eklmreco = register_module('EKLMReconstruction')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtmetagen)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)

main.add_module(eklmdigi)
# main.add_module(eklmreco)

main.add_module(simpleoutput)

main.add_module(mcprint)

# Process 100 events
process(main)

