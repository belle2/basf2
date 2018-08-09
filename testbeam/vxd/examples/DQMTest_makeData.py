#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2016 @ DESY Simulation
# This is the default simulation scenario for VXD beam test WITHOUT telescopes
#
# This steering file prepares simulation data for the SVD DQM module.
# Magnetic field is off.

# Important parameters of the simulation:
events = 1000  # Number of events to simulate
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # # degrees (sigma of gaussian)
phi = 0.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = -100.  # cm / 100cm, outside magnet, shielding, Al scatterer
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

from basf2 import *
# suppress messages and warnings during processing:
set_log_level(LogLevel.WARNING)
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
# Al target at 750mm to simulate 15m air between collimator and TB setup
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
# VXD (no Telescopes) plus the real PCMAG magnetic field
# gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB.xml')
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB2016.xml')

# Create geometry
geometry = register_module('Geometry')
# geometry = register_module('Geometry')
# No magnetic field
# geometry.param('components', ['TB'])
if not fieldOn:
    # To turn off magnetic field:
    geometry.param('excludedComponents', ['MagneticField'])

# Full simulation module
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', True)

# PXD/SVD/Tel digitizer
PXDDigi = register_module('PXDDigitizer')
# PXDDigi.param('SimpleDriftModel', False)

SVDDigi = register_module('SVDDigitizer')

TelDigi = register_module('TelDigitizer')
# TelDigi.param('ElectronicNoise', 200)
# TelDigi.param("NoiseSN", 5.0)

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulation.root')
# Save only digits for the DQM test.
# output.param('branchNames', ['PXDRawHits', 'SVDDigits', 'TelDigits'])

# Path construction
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDigi)
main.add_module(SVDDigi)
main.add_module(TelDigi)
main.add_module(output)
# Process events
process(main)

# Print call statistics
print(statistics)
