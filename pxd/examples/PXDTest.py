#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##############################################################################
#
# This is an example steering file to run the PXD part of Belle2 simulation.
# It uses ParicleGun module to generate tracks,
# (see "generators/example/ParticleGunFull.py" for detailed usage)
# builds PXD geometry, performs geant4 and PXD simulation,
# and stores output in a root file.
#
##############################################################################

import basf2 as b2

# show warnings during processing
b2.set_log_level(b2.LogLevel.WARNING)

# Register modules

# Particle gun module
particlegun = b2.register_module('ParticleGun')
# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
# Show progress of processing
progress = b2.register_module('Progress')
# Load parameters
gearbox = b2.register_module('Gearbox')
# Create geometry
geometry = b2.register_module('Geometry')
# Run simulation
simulation = b2.register_module('FullSim')
# PXD digitization module
PXDDIGI = b2.register_module('PXDDigitizer')
# PXD clusterizer
PXDCLUST = b2.register_module('PXDClusterizer')
# Save output of simulation
output = b2.register_module('RootOutput')

# ============================================================================
# Set a fixed random seed for particle generation:
b2.set_random_seed(1028307)

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-11, 11])

# ============================================================================
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)

# ============================================================================
# Set the number of events to be processed (10 events)
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

# Set output filename
output.param('outputFileName', 'PXDTestOutput.root')

# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD'])

PXDDIGI.set_log_level(b2.LogLevel.INFO)

PXDCLUST.set_log_level(b2.LogLevel.INFO)

# ============================================================================
# Do the simulation

main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)

main.add_module(output)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
#
