#!/usr/bin/env python
# -*- coding: utf-8 -*-

# TestBeam 2009 Simulation & Analysis
# Second simulation scenario with 120 GeV/c pions and Belle 2 50x50 um DUT
from basf2 import *
# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
# ParticleGun
particlegun = register_module('ParticleGun')
# pions:
particlegun.param('pdgCodes', [211, -211])
# number of primaries per event
particlegun.param('nTracks', 1)
# momentum 120 GeV/c
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [120, 120])
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [0.0, 0.1])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [0.0, 360])
# gun displacement according to the area of DUT
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [-0.256 * 0.8, 0.256 * 0.8])  # x OK
particlegun.param('yVertexParams', [-0.064 * 0.8, 0.064 * 0.8])  # y OK
particlegun.param('zVertexParams', [-1.0, 0.0])
particlegun.param('independentVertices', True)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')
# Load parameters from xml - load the geometry file which uses 50x50 um matrix
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/TB09Belle2.xml')

# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# Uncomment following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])

# Add PXD Digitizer
digit = register_module('PXDDigitizer')
digit.param('SimpleDriftModel', False)
digit.param('statisticsFilename', 'TB2009PXDDigiStat.root')

# Add PXD Clusterizer
cluster = register_module('PXDClusterizer')
# Turn off Magnetic field effets assumption during clustering
cluster.param('TanLorentz', 0.0)
cluster.param('AssumeSorted', False)

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulationOutput.root')

geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'TBGeometry.root')

# Path construction
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(geosaver)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(digit)
main.add_module(cluster)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
