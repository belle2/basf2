#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <output>EKLMK0LOutput.root</output>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Generation of 500 muons for EKLM validation.</description>
</header>
"""

import os
import random
from basf2 import *

set_log_level(LogLevel.WARNING)

# Fixed random seed
set_random_seed(12345)

# Event data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [500])
eventinfosetter.param('runList', [1])

# Particle gun
particlegun = register_module('ParticleGun')
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [130])
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1, 1])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [30, 30])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

# Geometry and Geant simulation
paramloader = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('components', ['EKLM'])
g4sim = register_module('FullSim')

# EKLM Modules
eklmDigitizer = register_module('EKLMDigitizer')
eklmReconstructor = register_module('EKLMReconstructor')
eklmK0LReconstructor = register_module('EKLMK0LReconstructor')
mc_matcher = register_module('MCMatcherKLMClusters')

# Output
output = register_module('RootOutput')
output.param('outputFileName', '../EKLMK0LOutput.root')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geometry)
main.add_module(g4sim)

main.add_module(eklmDigitizer)
main.add_module(eklmReconstructor)
main.add_module(eklmK0LReconstructor)
main.add_module(mc_matcher)

main.add_module(output)

# Process 100 events
process(main)
