#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <output>KLMK0LOutput.root</output>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Generation of 1000 B -> J/psi K_L0 events for KLM validation.</description>
</header>
"""

import os
import random
from basf2 import *
from ROOT import Belle2
from beamparameters import add_beamparameters

set_log_level(LogLevel.WARNING)

# Fixed random seed
set_random_seed(123456)

# Create main path
main = create_path()

# Event data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])

# Evtgen and beam parameters.
evtgen = register_module('EvtGenInput')
evtgen.param('userDECFile', Belle2.FileSystem.findFile('klm/validation/btojpsikl0.dec'))
beamparameters = add_beamparameters(main, "Y4S")

# Geometry and Geant simulation
paramloader = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('components', ['BKLM', 'EKLM'])
geometry.param('useDB', False)
g4sim = register_module('FullSim')

# KLM modules.
klmDigitizer = register_module('KLMDigitizer')
bklmReconstructor = register_module('BKLMReconstructor')
eklmReconstructor = register_module('EKLMReconstructor')
klmK0LReconstructor = register_module('KLMK0LReconstructor')
mc_matcher = register_module('MCMatcherKLMClusters')

# Output
output = register_module('RootOutput')
output.param('outputFileName', '../KLMK0LOutput.root')

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(evtgen)
main.add_module(paramloader)
main.add_module(geometry)
main.add_module(g4sim)

main.add_module(klmDigitizer)
main.add_module(bklmReconstructor)
main.add_module(eklmReconstructor)
main.add_module(klmK0LReconstructor)
main.add_module(mc_matcher)

main.add_module(output)

# Process 100 events
process(main)
