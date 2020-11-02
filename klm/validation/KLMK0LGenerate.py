#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <output>KLMK0LOutput.root</output>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Generation of 1000 B -> J/psi K_L0 events for KLM validation.</description>
</header>
"""

import basf2 as b2
from ROOT import Belle2
from beamparameters import add_beamparameters

b2.set_log_level(b2.LogLevel.WARNING)

# Fixed random seed
b2.set_random_seed(123456)

# Create main path
main = b2.create_path()

# Event data
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])

# Evtgen and beam parameters.
evtgen = b2.register_module('EvtGenInput')
evtgen.param('userDECFile', Belle2.FileSystem.findFile('klm/validation/btojpsikl0.dec'))
beamparameters = add_beamparameters(main, "Y4S")

# Geometry and Geant simulation
paramloader = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
geometry.param('components', ['KLM'])
geometry.param('useDB', False)
g4sim = b2.register_module('FullSim')

# KLM modules.
klmDigitizer = b2.register_module('KLMDigitizer')
klmReconstructor = b2.register_module('KLMReconstructor')
klmClustersReconstructor = b2.register_module('KLMClustersReconstructor')
mc_matcher = b2.register_module('MCMatcherKLMClusters')

# Add progress bars
progress = b2.register_module('Progress')
progressBar = b2.register_module('ProgressBar')

# Output
output = b2.register_module('RootOutput')
output.param('outputFileName', '../KLMK0LOutput.root')

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(evtgen)
main.add_module(paramloader)
main.add_module(geometry)
main.add_module(g4sim)

main.add_module(klmDigitizer)
main.add_module(klmReconstructor)
main.add_module(klmClustersReconstructor)
main.add_module(mc_matcher)

main.add_module(progress)
main.add_module(progressBar)

main.add_module(output)

# Process the path
b2.process(main)
