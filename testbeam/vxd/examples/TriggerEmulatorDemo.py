#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This script illustrates the usage of the TriggerEmulator module. Only those
# events fullfilling the trigger criterea will be shown in the event display.


# Important parameters of the simulation:
events = 250  # Number of events to simulate
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
# geometry.param('excludedComponents', ['MagneticField'])

# Full simulation module
simulation = register_module('FullSim')
simulation.logging.log_level = LogLevel.ERROR
# and make it store the particle trajectories so we can draw them
simulation.param('trajectoryStore', 2)


# we need the Telscope clusters
TelDigi = register_module('TelDigitizer')
TelClust = register_module('TelClusterizer')


display = register_module('Display')
# The Options parameter is a combination of:
# D draw detectors - draw simple detector representation (with different size)
#   for each hit
# H draw track hits
# M draw track markers - intersections of track with detector planes
#   (use with T)
# P draw detector planes
# S scale manually - spacepoint hits are drawn as spheres and scaled with
#   errors
# T draw track (straight line between detector planes)
#
# Note that you can always turn off an individual detector component or track
# interactively by removing its checkmark in the 'Eve' tab.
#
# This option only makes sense when ShowGFTracks is true
display.param('options', 'HTM')  # default

# should hits always be assigned to a particle with c_PrimaryParticle flag?
# with this option off, many tracking hits will be assigned to secondary e-
display.param('assignHitsToPrimaries', 0)

# show all primary MCParticles?
display.param('showAllPrimaries', True)

# show all charged MCParticles? (SLOW)
display.param('showCharged', False)

# show tracks?
display.param('showTrackLevelObjects', True)

# save events non-interactively (without showing window)?
display.param('automatic', False)

# Use clusters to display tracks
display.param('useClusters', True)

# Display the testbeam geometry rather than Belle II extract
display.param('fullGeometry', True)


TriggerEmu = register_module('TriggerEmulator')
TriggerEmu.param('inputArrayName', '')  # this is the default!
# you have to specify the sensors which should be used to trigger!
# Scintilator ids are: 7.1.1   7.1.2   7.1.3
# telescope ids are: 7.2.1  7.2.2  7.2.3  7.3.4  7.3.5  7.3.6
# (these are the ids as defined in the geometry file!)
TriggerEmu.param('sensorList', '7.1.1 7.1.2 7.1.3')
TriggerEmu.logging.log_level = LogLevel.DEBUG


# Create a path to be executed only if triggered! Put everything you
triggerpath = create_path()  # empty path!
# Tell the trigger the abort condition (in this case at least 2 hits). At abort it will execute the EMPTY path and end the event.
# The return value of the module is the number of sensors having at least one cluster reconstructed. Only
# sensors listed in the "sensorList" parameter are considered!
TriggerEmu.if_value('<2', triggerpath, AfterConditionPath.END)  # note that this will end the event!


# --- ALTERNATIVE VERSION: this version creates an alternative path to be executed if triggered
# ---                      (this is complementary to the above version as the main path is continued afterwards!)
# triggerpath.add_module(display)
# --- tell the trigger module to execute the alternative path if condition is met (in this case at least two hits)
# TriggerEmu.if_value('>=2', triggerpath, AfterConditionPath.CONTINUE)


# Create main paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(TelDigi)
main.add_module(TelClust)

main.add_module(TriggerEmu)
# Note that the trigger module with the setting above will end the path if the condition is met!
# So no module after it will be executed!
main.add_module(display)


# Process events
process(main)

print(statistics)
