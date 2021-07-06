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
# This is an example steering file to create an output file 'PXDRawHit.root'
# containing branches PXDDigits and RawPXD. Use this file to test unpacking
# and DQM modules in examples/unpacking.
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
# Convert digits to raw pxd data
PXDPACKER = b2.register_module('PXDPacker')
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
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# ============================================================================
# Set output filename
output.param('outputFileName', 'PXDRawHit.root')

# ============================================================================
# [[dhhc1, dhh1, dhh2, dhh3, dhh4, dhh5] [ ... ]]
# -1 is disable port
PXDPACKER.param('dhe_to_dhc', [
    [0, 2, 4, 34, 36, 38],
    [1, 6, 8, 40, 42, 44],
    [2, 10, 12, 46, 48, 50],
    [3, 14, 16, 52, 54, 56],
    [4, 3, 5, 35, 37, 39],
    [5, 7, 9, 41, 43, 45],
    [6, 11, 13, 47, 49, 51],
    [7, 15, 17, 53, 55, 57],
])

# ============================================================================
# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD'])

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
main.add_module(PXDPACKER)
main.add_module(output)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
#
