#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Oct 2013 @ DESY Simulation
# This is the default simulation scenario
from basf2 import *
# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
# ParticleGun
particlegun = register_module('ParticleGun')

# number of primaries per event
particlegun.param('nTracks', 1)

# DESY electrons:
particlegun.param('pdgCodes', [11])
# momentum magnitude 2 GeV/c or something above or around.
# At DESY we can 6 GeV/c(+-5%) electron beam.
# Beam divergence 2mrad not covered yet (we need some starting point location)
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [6.0, 0.0])
# momentum direction
particlegun.param('thetaGeneration', 'fixed')
particlegun.param('thetaParams', [90.0, 0.0])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [0.0, 0.0])
# gun displacement
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [-80.0, 0.0])
particlegun.param('yVertexParams', [0.0, 0.0])
particlegun.param('zVertexParams', [0.0, 0.0])
particlegun.param('independentVertices', True)

# Create Event information
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'EvtNumList': [20000], 'RunList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# This file contains the VXD beam test geometry including the magnetic field
gearbox.param('Filename', 'testbeam/VXDBeamProfile.xml')

# Create geometry
geometry = register_module('Geometry')

# Run simulation
simulation = register_module('FullSim')

# Uncomment the following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                '/vis/scene/add/axes 0 0 0 100 mm',
#                '/vis/scene/add/trajectories smooth',
#                '/vis/modeling/trajectories/create/drawByCharge'])

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulationOutputBeamProfNoTarget.root')
# output.param('branchNames[0]', 'VXDHits')

geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'TBGeometry.root')

trueinfo = register_module('VXDHit')

# Save output of simulation
inputm = register_module('RootInput')
inputm.param('inputFileName', 'TBSimulationOutput.root')

# Path construction
main = create_path()
main.add_module(evtmetagen)
# main.add_module(inputm)

main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(trueinfo)
main.add_module(output)
main.add_module(geosaver)

# Process events
process(main)

# Print call statistics
print statistics
