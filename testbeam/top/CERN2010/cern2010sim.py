#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# CERN beam test 2010 using 120 GeV pion beam

# beam definition for reference runs (71 - 268)

beamRef = {
    'pdgCodes': [211],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'fixed',
    'momentumParams': [120],
    'thetaGeneration': 'normal',
    'thetaParams': [90, 0],
    'phiGeneration': 'normal',
    'phiParams': [0, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [0, 0.5],
    'zVertexParams': [87.5, 0.5],
    }

# beam definition for cosTheta=0.5 runs (269 - 288)

beam05 = {  #          'thetaParams': [60, 0],
            #          'phiParams': [0, 0],
            #          'zVertexParams': [78, 0.58]
    'pdgCodes': [211],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'fixed',
    'momentumParams': [120],
    'thetaGeneration': 'normal',
    'thetaParams': [59.64, 0],
    'phiGeneration': 'normal',
    'phiParams': [-0.29, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [0, 0.5],
    'zVertexParams': [76.21, 0.58],
    }

# beam definition for cosTheta=0.3 runs (289 - 306)

beam03 = {
    'pdgCodes': [211],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'fixed',
    'momentumParams': [120],
    'thetaGeneration': 'normal',
    'thetaParams': [72.54, 0],
    'phiGeneration': 'normal',
    'phiParams': [0, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [0, 0.5],
    'zVertexParams': [78, 0.52],
    }

# beam definition for cosTheta=0.0 runs (307 - 312)

beam00 = {
    'pdgCodes': [211],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'fixed',
    'momentumParams': [120],
    'thetaGeneration': 'normal',
    'thetaParams': [90, 0],
    'phiGeneration': 'normal',
    'phiParams': [0, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [0, 0.5],
    'zVertexParams': [78, 0.5],
    }

# beam definition for cosTheta=0.0, x=-15 cm runs (307 - 312)

beam00_x15 = {
    'pdgCodes': [211],
    'nTracks': 1,
    'varyNTracks': False,
    'momentumGeneration': 'fixed',
    'momentumParams': [120],
    'thetaGeneration': 'normal',
    'thetaParams': [90, 0],
    'phiGeneration': 'normal',
    'phiParams': [0, 0],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 0],
    'yVertexParams': [15, 0.5],
    'zVertexParams': [78, 0.5],
    }

# specify here beam setup, number of events to simulate and output file
# -------------------------------------------------------------------------
beam = beam05
nevents = [100]
outfile = 'TOPbeamtest05.root'
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
gearbox.param('fileName', 'testbeam/top/CERN2010/TOP.xml')

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])

# Simulation
simulation = register_module('FullSim')

# TOP digitization
TOPdigi = register_module('TOPDigitizer')
TOPdigi.param('electronicJitter', 0)
TOPdigi.param('timeZeroJitter', 21.5e-3)

# Output
output = register_module('TOPCern2010Output')
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
