#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# LEPS beam test 2013-june using 2.1 GeV/c e+ beam

# beam definition for cosTheta=0.0 runs

beam00 = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'normal',
    'thetaParams': [89.83, 0],
    'phiGeneration': 'normal',
    'phiParams': [0, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [-0.4, 0.5],
    'zVertexParams': [122.8, 0.5],
    }

# beam definition for cosTheta=0.4 runs (tentatively)

beam04 = {
    'pdgCodes': [-11],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'normal',
    'momentumParams': [2.12, 0.21],
    'thetaGeneration': 'normal',
    'thetaParams': [66.4, 0],
    'phiGeneration': 'normal',
    'phiParams': [0, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [-0.4, 0.5],
    'zVertexParams': [122.8, 0.5],
    }

# specify here beam setup, number of events to simulate and output file
# -------------------------------------------------------------------------
beam = beam04
nevents = [100]
outfile = 'LEPSbeamtest04.root'
# -------------------------------------------------------------------------

# Number of events to generate
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'evtNumList': nevents, 'runList': [1]})

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
geometry.param('Components', ['TOP'])

# Simulation
simulation = register_module('FullSim')

# TOP digitization
TOPdigi = register_module('TOPDigitizer')
TOPdigi.param('electronicJitter', 100e-3)
TOPdigi.param('timeZeroJitter', 25e-3)

# Output
output = register_module('RootOutput')
output.param('outputFileName', outfile)

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()
main.add_module(evtmetagen)
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
