#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# LEPS beam test 2013-june using 2.1 GeV/c e+ beam

# Belle coordinate frame is used: bar along z-axis, perpendicular to x-axis at x=120cm
# beam fired from x=0
#
#   x = z(LEPS)+120 cm
#   y = -y(LEPS)
#   z = x(LEPS)
#

# beam definition for experiment 2 (cosTheta=0.0)

beam02 = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'normal',
    'thetaParams': [90.525, 0],
    'phiGeneration': 'normal',
    'phiParams': [0, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [0.5, 0.5],
    'zVertexParams': [125.25, 0.5],
    }

# beam definition for experiment 3 (cosTheta=0.43)

beam03 = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'normal',
    'thetaParams': [64.80, 0],
    'phiGeneration': 'normal',
    'phiParams': [0.024, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [1.31, 0.5],
    'zVertexParams': [55.40, 0.5],
    }

# beam definition for experiment 4 (cosTheta=0.36)

beam04 = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'normal',
    'thetaParams': [68.35, 0],
    'phiGeneration': 'normal',
    'phiParams': [0.90, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [-19.90, 0.5],
    'zVertexParams': [63.80, 0.5],
    }

# specify here the beam setup, number of events to simulate and output file
# -------------------------------------------------------------------------
beam = beam02
tracker = 'TriggerTelescopeExp2'
nevents = [100]
outfile = 'LEPSbeamtest02.root'
# -------------------------------------------------------------------------

# Number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': nevents, 'runList': [1]})

# particle gun
particlegun = register_module('ParticleGun')
particlegun.param(beam)
print_params(particlegun)

# Show progress of processing
progress = register_module('Progress')

# Gearbox
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/top/LEPS2013-6/TOP.xml')

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP', tracker])

# Simulation
simulation = register_module('FullSim')

# TOP digitization
TOPdigi = register_module('TOPDigitizer')
TOPdigi.param('electronicJitter', 100e-3)
TOPdigi.param('timeZeroJitter', 25e-3)

# Output
# output = register_module('RootOutput')
output = register_module('TOPLeps2013Output')
output.param('outputFileName', outfile)

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(TOPdigi)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
