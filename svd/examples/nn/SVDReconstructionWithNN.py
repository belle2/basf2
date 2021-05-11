#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This steering file demonstrates the use of the new SVD reconstruction path.
# Currently it ends by creating SVDRecoDigits, clustering is not yet in place.
#
##############################################################################

import basf2 as b2
from svd.dump_digits import dump_digits
from svd.dump_clusters import dump_clusters

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
# Add the PXD digitizer, too, to avoid problems with empty events.
PXDDIGI = b2.register_module('PXDDigitizer')
# SVD digitization module
SVDDIGI = b2.register_module('SVDDigitizer')
SVDDIGI.param('StartSampling', -31.44)
SVDDIGI.param('GenerateShaperDigits', True)
# SVDDIGI.param('signalsList', 'SVDSignalsList.csv')
# SVDDIGI.param('RandomizeEventTimes', True)
# SVDDIGI.param('TimeFrameLow', -180)
# SVDDIGI.param('TimeFrameHigh', 150)
# SVD signal reconstructor
SVDSIGR = b2.register_module('SVDNNShapeReconstructor')
# Write RecoDigits so that we can check them
SVDSIGR.param('WriteRecoDigits', True)
# RecoDigit clusterizer
SVDCLUST1 = b2.register_module('SVDNNClusterizer')
# ShaperDigit clusterizer
SVDCLUST2 = b2.register_module("SVDClusterizerDirect")
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

# Set the number of events to be processed (100 events)
eventinfosetter.param({'evtNumList': [1000], 'expList': [0], 'runList': [0]})

# Set output filename
output.param('outputFileName', 'SVDTestOutput.root')

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

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
main.add_module(SVDDIGI)
main.add_module(SVDSIGR)
main.add_module(dump_digits())
main.add_module(SVDCLUST2)
main.add_module(dump_clusters())
main.add_module(output)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
