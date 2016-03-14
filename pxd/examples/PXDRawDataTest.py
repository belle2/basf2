#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to run the PXD part of Belle2 simulation.
# to test the real data processing part.
#
##############################################################################

from basf2 import *

# show warnings during processing
set_log_level(LogLevel.WARNING)

# Register modules

# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# PXD digitization module
PXDDIGI = register_module('PXDDigitizer')
# Convert digits to PXDRawHits
rawhitmaker = register_module('PXDRawHitProducer')
# ... and back
rawhitsorter = register_module('PXDRawHitSorter')
rawhitsorter.param('mergeFrames', False)
# PXD clusterizer
PXDCLUST = register_module('PXDClusterizer')
# Save output of simulation
output = register_module('RootOutput')

# ============================================================================
# Set a fixed random seed for particle generation:
set_random_seed(1028307)

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-11, 11])

# ============================================================================
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)

# ============================================================================

# Set the number of events to be processed (10 events)
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# Set output filename
output.param('outputFileName', 'PXDTestOutput.root')

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('components', ['PXD'])

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(PXDDIGI)
main.add_module(rawhitmaker)
main.add_module(rawhitsorter)
main.add_module(PXDCLUST)

main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
#
