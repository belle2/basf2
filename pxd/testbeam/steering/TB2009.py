#!/usr/bin/env python
# -*- coding: utf-8 -*-

# TestBeam 2009 Simulation & Analysis
# This is the default simulation scenario with 120 GeV/c pions and ILC 20x20 um DUT
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
# sensors in the geometry are placed into a beamline - at theta=0, phi=0-360 in Belle2 coordinate system
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [0.0, 1.2])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [0.0, 360])
# gun displacement according to the area of DUT (detector under test - sensor_id=3) times 0.8
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [-0.256 * 0.8, 0.256 * 0.8])  # x OK
particlegun.param('yVertexParams', [-0.064 * 0.8, 0.064 * 0.8])  # y OK
particlegun.param('zVertexParams', [-1.0, 0.0])
particlegun.param('independentVertices', True)

# Create Event information
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'EvtNumList': [100], 'RunList': [1]})

# Show progress of processing
progress = register_module('Progress')
# Load parameters from xml
gearbox = register_module('Gearbox')
gearbox.param('Filename', 'pxd/testbeam/TB09-Geometry.xml')

# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# Uncomment the following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])

# Add DUT Digitizer
DUT_digi = register_module('PXDDigitizer')
# Turn off Magnetic field effects during digitization
DUT_digi.param('SimpleDriftModel', False)
DUT_digi.param('statisticsFilename', 'TB2009PXDDigiStat.root')
# Correct Electronic noise for ILC matrix
DUT_digi.param('ElectronicNoise', 290)
# Add PXD Clusterizer
DUT_clust = register_module('PXDClusterizer')
# Turn off Magnetic field effects assumption during clustering
DUT_clust.param('TanLorentz', 0.0)
DUT_clust.param('AssumeSorted', False)
# Correct Electronic noise for ILC matrix
DUT_clust.param('ElectronicNoise', 290)
DUT_clust.set_log_level(LogLevel.INFO)
# Add Telescope Smearing Digitizer/Clusterizer
TEL_digi = register_module('TelDigiCluster')

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulationOutput.root')

# Path construction
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(DUT_digi)
main.add_module(DUT_clust)
main.add_module(TEL_digi)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
