#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2014 @ DESY Simulation
# This is the default simulation scenario for VXD beam test WITHOUT telescopes

# Important parameters of the simulation:
events = 10000  # Number of events to simulate
fieldOn = False  # Turn field on or off (changes geometry components and digi/clust params)
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # # degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

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
# At DESY we can have up to 6 GeV/c(+-5%) electron beam.
# Beam divergence divergence and spot size is adjusted similar to reality
# See studies of Benjamin Schwenker
particlegun.param('momentumGeneration', 'normal')
particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
# momentum direction must be around theta=90, phi=180
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
# gun position must be in positive values of x.
# Magnet wall starts at 424mm and ends at 590mm
# Plastic 1cm shielding is at 650mm
# Aluminium target at 750mm to "simulate" 15m air between collimator and TB setup
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.])
particlegun.param('yVertexParams', [0., beamspot_size_y])
particlegun.param('zVertexParams', [0., beamspot_size_z])
particlegun.param('independentVertices', True)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# This file contains the VXD (no Telescopes) beam test geometry including the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB.xml')

# Create geometry
geometry = register_module('Geometry')
# You can specify components to be created
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
else:
  # To turn off magnetic field:
    geometry.param('components', ['TB'])

# Full simulation module
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', True)
# Uncomment the following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])

# PXD/SVD digitizer
PXDDigi = register_module('PXDDigitizer')
# turn off Lorentz angle simulation if no field
if fieldOn:
    PXDDigi.param('tanLorentz', 0.)
else:
    PXDDigi.param('tanLorentz', 0.1625)  # value scaled from 0.25 for 1.5T to 0.975T

# PXDDigi.param('SimpleDriftModel', False)

SVDDigi = register_module('SVDDigitizer')

# PXD/SVD clusterizer
PXDClust = register_module('PXDClusterizer')
# ClusterCacheSize must be changed to minimum 576 (number of columns) for EUDET matrices (576x1152) to work.
# Otherwise you will get 'segmentation violation'.
PXDClust.param('ClusterCacheSize', 576)

if fieldOn:
    PXDClust.param('TanLorentz', 0.)
else:
    PXDClust.param('TanLorentz', 0.1625)  # value scaled from 0.25 for 1.5T to 0.975T

SVDClust = register_module('SVDClusterizer')
if fieldOn:
    SVDClust.param('TanLorentz_holes', 0.)
    SVDClust.param('TanLorentz_electrons', 0.)
else:
    SVDClust.param('TanLorentz_holes', 0.052)  # value scaled from 0.08 for 1.5T to 0.975T
    SVDClust.param('TanLorentz_electrons', 0.)

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulation.root')
# You can specify branch names to be saved (only), see module doc
# output.param('branchNames[0]', ['PXDTrueHits', 'SVDTrueHits'])

# Export used geometry for checking
geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'TBGeometry.root')

# Use truth information to create track candidates
mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.logging.log_level = LogLevel.WARNING
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    'WhichParticles': ['SVD'],
    }
mctrackfinder.param(param_mctrackfinder)

# mctrackfinder.logging.log_level = LogLevel.DEBUG

# Fit tracks with GBLfit which produces millepede.dat for alignment
gblfitter = register_module('GBLfit')
gblfitter.logging.log_level = LogLevel.WARNING
gblfitter.param('UseClusters', True)
# gblfitter.param('FilterId', 'GBL')

# Check track fitting results
trackfitchecker = register_module('TrackFitChecker')
# the results only show up at info or debug level
trackfitchecker.logging.log_level = LogLevel.INFO
# trackfitchecker.param('inspectTracks', True)
trackfitchecker.param('truthAvailable', True)
# trackfitchecker.param('testSi', True)
trackfitchecker.param('robustTests', True)
trackfitchecker.param('writeToRootFile', True)

display = register_module('Display')
display.param('fullGeometry', True)

# Path construction
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDigi)
# This module changes specified PXD sensors to binary readout: EUDET emulation
# It must be before PXDClusterizer in the path!
EUDET_emulator = register_module('ChangePXDDigitsToBinary')
main.add_module(EUDET_emulator)

main.add_module(SVDDigi)
main.add_module(PXDClust)
main.add_module(SVDClust)
main.add_module(mctrackfinder)
main.add_module(gblfitter)
main.add_module(trackfitchecker)
main.add_module(geosaver)
main.add_module(output)
main.add_module(display)
# Alignment with Millepede II. Will use steer.txt file for MP2 steering,
# which links also automatically generated file with geometry constraints constraints.txt
alignment = register_module('MillepedeIIalignment')
main.add_module(alignment)
# Process events
process(main)

# Print call statistics
print statistics
