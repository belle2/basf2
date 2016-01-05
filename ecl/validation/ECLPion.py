#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<output>ECLPionOutput.root</output>
<contact>Benjamin Oberhof, ecl2ml@bpost.kek.jp</contact>
</header>
"""

import os
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_log_level(LogLevel.ERROR)

# Register necessary modules
eventinfosetter = register_module('EventInfoSetter')
# Set the number of events to be processed (1000 events)
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

eventinfoprinter = register_module('EventInfoPrinter')

# Fixed random seed
set_random_seed(123456)

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [211],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [1],
    'thetaGeneration': 'uniform',
    'thetaParams': [0., 180.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0., 0.],
    'yVertexParams': [0., 0.],
    'zVertexParams': [0., 0.],
    }
pGun.param(param_pGun)

# simpleoutput = register_module('RootOutput')
# simpleoutput.param('outputFileName', '../ECLPionOutput.root')
eclanalysis = register_module('ECLDataAnalysis')
eclanalysis.param('rootFileName', '../ECLPionOutput.root')
eclanalysis.param('doTracking', 1)

# Create paths
main = create_path()
main.add_module(eventinfosetter)
# main.add_module(eventinfoprinter)
main.add_module(pGun)
add_simulation(main)
add_reconstruction(main)
main.add_module(eclanalysis)
# main.add_module(simpleoutput)

process(main)
# print(statistics)
