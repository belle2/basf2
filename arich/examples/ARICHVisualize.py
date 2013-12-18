#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to visulize ARICH subdetector.
# It creates .wrl file which can be opened with apropriate
# viewer (ex. freewrl).
#
##############################################################################

from basf2 import *

# suppress messages and warnings during processing DEBUG, INFO, WARNING, ERROR
set_log_level(LogLevel.WARNING)

# to run the framework the used modules need to be registered

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Particle gun module
particlegun = register_module('ParticleGun')
# Run simulation
simulation = register_module('FullSim')

# EventInfoSetter parameters
# Set the number of events to be processed (10 event)
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
# ============================================================================

# ============================================================================
# ParticleGun parameters
# Setting the random seed for particle generation:
set_random_seed(1028307)
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-211, 211])
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)
# if you set nTracks to 0, then for each PDG code in pdgCodes list a track
# will be generated on each event.
# Setting the parameters for the random generation
# of particles momenta:
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1, 4])
# Setting the parameters for the random generation
# of the particle polar angle:
particlegun.param('thetaGeneration', 'uniformCosinus')
particlegun.param('thetaParams', [17, 35])
# Print the parameters of the particle gun
print_params(particlegun)
# ============================================================================

# ============================================================================
# Geometry parameters
# Select subdetectors to be built
geometry.param('Components', ['ARICH'])
# If you comment this out all subdetectors will be built. If you want to
# include just some of them do for example ['ARICH','TOP','CDC'].
# ============================================================================

# ============================================================================
# Simulation parameters
# To speed up the simulation you can propagate only a selected fraction of
# photons.
simulation.param('PhotonFraction', 0.3)
# !!! NOTE: if you use ARICH digitization module this must be set to 0.3
# since HAPD q.e. curve is scaled to that value
# (will be solved in one of next releases)
# Here you can select visualization driver and visualization commands.
# You can use any visualization supported by geant4:
# "http://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/
#  ForApplicationDeveloper/html/ch08.html"
# Uncomment following lines  to create VRML file that can be examined with any
# vrml viewer (freewrl,...)
simulation.param('EnableVisualization', True)
simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
                 '/vis/scene/add/axes 0 0 0 100 mm',
                 '/vis/scene/add/trajectories smooth',
                 '/vis/modeling/trajectories/create/drawByCharge'])
# =============================================================================

# Do the simulation
# =============================================================================
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)

# Process events
process(main)

# Print call statistics
print statistics
