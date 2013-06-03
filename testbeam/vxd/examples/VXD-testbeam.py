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
particlegun.param('nTracks', 10)

# DESY electrons:
particlegun.param('pdgCodes', [11])
# momentum magnitude 2 GeV/c or something above or around.
# At DESY we can 6 GeV/c(+-5%) electron beam.
# Beam divergence 2mrad not covered yet (we need some starting point location)
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [2.0, 0.0])
# momentum direction
particlegun.param('thetaGeneration', 'fixed')
particlegun.param('thetaParams', [90.0, 0.0])
particlegun.param('phiGeneration', 'fixed')
particlegun.param('phiParams', [0.0, 0.0])
# gun displacement
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [-90.0, 0.0])
particlegun.param('yVertexParams', [0.0, 0.0])
particlegun.param('zVertexParams', [0.0, 0.0])
particlegun.param('independentVertices', True)

# Create Event information
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'evtNumList': [100], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# This file contains the VXD beam test geometry including the magnetic field
gearbox.param('fileName', 'testbeam/vxd/VXD.xml')

# Create geometry
geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'TB'])
# Run simulation
simulation = register_module('FullSim')

# Uncomment the following lines to get particle tracks visualization
simulation.param('EnableVisualization', True)
simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
                 '/vis/scene/add/axes 0 0 0 100 mm',
                 '/vis/scene/add/trajectories smooth',
                 '/vis/modeling/trajectories/create/drawByCharge'])
# PXD/SVD digitizer
PXDDigi = register_module('PXDDigitizer')
SVDDigi = register_module('SVDDigitizer')
# PXD/SVD clusterizer
PXDClust = register_module('PXDClusterizer')
SVDClust = register_module('SVDClusterizer')
# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulationOutput.root')
# output.param('branchNames[0]', 'VXDHits')

geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'TBGeometry.root')

# trueinfo = register_module('VXDHit')

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
# main.add_module(trueinfo)
main.add_module(PXDDigi)
main.add_module(SVDDigi)
main.add_module(PXDClust)
main.add_module(SVDClust)
main.add_module(output)
main.add_module(geosaver)

# Process events
process(main)

# Print call statistics
print statistics
