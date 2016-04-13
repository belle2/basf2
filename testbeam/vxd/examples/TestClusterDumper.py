#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# a simple script to test the module which dumps cluster information to a root file

# Important parameters of the simulation:
events = 100  # Number of events to simulate
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # degrees (sigma of gaussian)
phi = 0.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = -100.  # cm ... 100cm ... outside magnet + plastic shielding
gun_y_position = 0.0
gun_z_position = 0.0
# + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

from basf2 import *

# ParticleGun
particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', 1)
# DESY electrons:
particlegun.param('pdgCodes', [11])
# momentum magnitude 2 GeV/c or something above or around.
# At DESY we can have up to 6 GeV/c(+-5%) electron beam.
# Beam divergence divergence and spot size is adjusted similar to reality
# See studies of Benjamin Schwenker
particlegun.param('momentumGeneration', 'normal')
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
# momentum direction must be around theta=90, phi=180
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
# gun position must be in positive values of x.
# Magnet wall starts at 424mm and ends at 590mm
# Plastic 1cm shielding is at 650mm
# Aluminium target at 750mm to "simulate" 15m air between collimator and TB
# setup
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.0])
particlegun.param('yVertexParams', [gun_y_position, beamspot_size_y])
particlegun.param('zVertexParams', [gun_z_position, beamspot_size_z])
particlegun.param('independentVertices', True)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# This file contains the VXD (no Telescopes) beam test geometry including
# the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB2016.xml')

# Create geometry
geometry = register_module('Geometry')
# You can specify components to be created
# To turn off magnetic field:
geometry.param('excludedComponents', ['MagneticField', 'Magnet'])

# Full simulation module
simulation = register_module('FullSim')
simulation.logging.log_level = LogLevel.ERROR
# and make it store the particle trajectories so we can draw them
simulation.param('trajectoryStore', 2)

# Uncomment the following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])

# PXD/SVD digitizer
PXDDigi = register_module('PXDDigitizer')
SVDDigi = register_module('SVDDigitizer')
# PXD/SVD clusterizer
PXDClust = register_module('PXDClusterizer')
# PXDClust.param('ClusterCacheSize', 576)
SVDClust = register_module('SVDClusterizer')

dumper = register_module('DumpClusters')
dumper.param('SVDClusters', '')
dumper.param('PXDClusters', '')
# dumper.logging.log_level = LogLevel.DEBUG

histo = register_module('HistoManager')
histo.param('histoFileName', 'testOut.root')  # File to save histograms


# Create paths
main = create_path()
# Add modules to paths

main.add_module(histo)

main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDigi)
main.add_module(SVDDigi)
main.add_module(PXDClust)
main.add_module(SVDClust)

# main.add_module(output)
main.add_module(dumper)

# Process events
process(main)

print(statistics)
