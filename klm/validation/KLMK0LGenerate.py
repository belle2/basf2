#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <output>KLMK0LOutput.root</output>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Generation of 1000 B -> J/psi K_L0 events for KLM validation.</description>
</header>
"""

import basf2
from beamparameters import add_beamparameters

basf2.set_log_level(basf2.LogLevel.WARNING)

# Fixed random seed
basf2.set_random_seed(123456)

# Create main path
main = basf2.create_path()

# Event data
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])

# Evtgen and beam parameters.
evtgen = basf2.register_module('EvtGenInput')
evtgen.param('userDECFile', basf2.find_file('klm/validation/btojpsikl0.dec'))
beamparameters = add_beamparameters(main, "Y4S")

# Geometry and Geant simulation
paramloader = basf2.register_module('Gearbox')
geometry = basf2.register_module('Geometry')
geometry.param('components', ['KLM'])
geometry.param('useDB', False)
g4sim = basf2.register_module('FullSim')

# KLM modules.
klmDigitizer = basf2.register_module('KLMDigitizer')
klmReconstructor = basf2.register_module('KLMReconstructor')
klmClustersReconstructor = basf2.register_module('KLMClustersReconstructor')
mc_matcher = basf2.register_module('MCMatcherKLMClusters')

# Add progress bars
progress = basf2.register_module('Progress')
progressBar = basf2.register_module('ProgressBar')

# Output
output = basf2.register_module('RootOutput')
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
basf2.process(main)
