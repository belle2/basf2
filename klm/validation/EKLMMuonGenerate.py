#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <output>EKLMMuonOutput.root</output>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Generation of 500 K_L0 for EKLM validation.</description>
</header>
"""

import basf2 as b2

b2.set_log_level(b2.LogLevel.WARNING)

# Fixed random seed
b2.set_random_seed(12345)

# Event data
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [500])

# Particle gun
particlegun = b2.register_module('ParticleGun')
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [13, -13])
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1, 1])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [30, 30])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

# Geometry and Geant simulation
paramloader = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
geometry.param('components', ['KLM'])
geometry.param('useDB', False)
g4sim = b2.register_module('FullSim')

# EKLM Modules
klmDigitizer = b2.register_module('KLMDigitizer')
klmReconstructor = b2.register_module('KLMReconstructor')

# Output
output = b2.register_module('RootOutput')
output.param('outputFileName', '../EKLMMuonOutput.root')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geometry)
main.add_module(g4sim)

main.add_module(klmDigitizer)
main.add_module(klmReconstructor)

main.add_module(output)

# Process 100 events
b2.process(main)
