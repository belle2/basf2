#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from PXDHitErrorsTTree import PXDHitErrorsTTree
b2.logging.log_level = b2.LogLevel.WARNING

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
pxddigi = b2.register_module('PXDDigitizer')
# PXD clustering module
pxdclust = b2.register_module('PXDClusterizer')
# RootOutput
output = b2.register_module('RootOutput')

analyze = PXDHitErrorsTTree()

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [10000], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({
    # Generate 1 track (always)
    'nTracks': 1,
    # don't vary the number according to Poisson distribution
    'varyNTracks': False,
    # Generate pi+, pi-, e+ and e-
    'pdgCodes': [211, -211, 11, -11],
    # with a normal distributed transversal momentum
    'momentumGeneration': 'normal',
    # with a center of 2 GeV and a width of 0.2 GeV
    'momentumParams': [2, 0.2],
    # a uniform distributed phi angle,
    'phiGeneration': 'uniform',
    # full circle
    'phiParams': [0, 360],
    # Generate theta angles uniform in cos theta
    'thetaGeneration': 'uniformCos',
    # between 30 and 150 degree
    'thetaParams': [30, 150],
    # normal distributed vertex generation
    'vertexGeneration': 'normal',
    # around the origin with a sigma of 0.5cm in each direction
    'xVertexParams': [0.0, 0.5],
    'yVertexParams': [0.0, 0.5],
    'zVertexParams': [0.0, 0.5],
    # all one track sharing the same vertex per event
    'independentVertices': False,
})

# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD'])

# pxddigi.param('statisticsFilename', 'digi.root')
pxddigi.param('ElectronicEffects', True)
pxddigi.param('SimpleDriftModel', False)

# create processing path
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(pxddigi)
main.add_module(pxdclust)
main.add_module(analyze)
# main.add_module(output)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
