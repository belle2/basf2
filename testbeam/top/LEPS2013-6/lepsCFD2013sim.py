#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# LEPS beam test 2013-june using 2.1 GeV/c e+ beam: runs with CFD electronics

# Belle coordinate frame is used: bar along z-axis, perpendicular to x-axis at x=120cm
# beam fired from x=0
#
#   x = z(LEPS) + 120 cm
#   y = -y(LEPS)
#   z = x(LEPS)
#
# Inami-san alignment data conversion:
#
#   x = y(Inami)
#   y = -x(Inami)
#   z = z(Inami) + 695 mm
#   theta = theta(Inami)
#   phi = -phi(Inami)/sin(theta(Inami))
# (last two in approximation for phi<<1)
# ------------------------------------------

# beam definition for exp001 (cosTheta=0.0), new alignment with basf2 (2013/9/5)

beam01 = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'normal',
    'thetaParams': [90.2, 0.10],
    'phiGeneration': 'normal',
    'phiParams': [0.0, 0.10],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [0.0, 0.15],
    'zVertexParams': [124.8, 0.15],
    }

# beam definition for exp002 (cosTheta=0.39), new alignment with basf2 (2013/9/5)

beam02 = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'normal',
    'thetaParams': [67.5, 0.10],
    'phiGeneration': 'normal',
    'phiParams': [0.0, 0.10],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [0.0, 0.15],
    'zVertexParams': [73.4, 0.15],
    }

# beam definition for exp003 (cosTheta=0.39, impact close to bar side)
# new alignment with basf2 (2013/9/5)

beam03 = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'normal',
    'thetaParams': [67.65, 0.10],
    'phiGeneration': 'normal',
    'phiParams': [0.10, 0.10],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [-19.2, 0.15],
    'zVertexParams': [74.6, 0.15],
    }

# specify here the beam setup, number of events to simulate and output file
# -------------------------------------------------------------------------
beam = beam01
nevents = [100]
outfile = 'LEPS-CFDbeamtest01.root'
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
gearbox.param('fileName', 'testbeam/top/LEPS2013-6/TOP-CFD.xml')

# Geometry
geometry = register_module('Geometry')
geometry.param('Components', ['TOP'])

# Simulation
simulation = register_module('FullSim')

# TOP digitization
TOPdigi = register_module('TOPDigitizer')
TOPdigi.param('electronicJitter', 35e-3)
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
