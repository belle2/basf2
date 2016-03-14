#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from SVDHitErrorsTTree import SVDHitErrorsTTree
logging.log_level = LogLevel.WARNING

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
# SVD digitization module
svddigi = register_module('SVDDigitizer')
# SVD clustering module
svdclust = register_module('SVDClusterizer')
# RootOutput
output = register_module('RootOutput')

analyze = SVDHitErrorsTTree()

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({  # Generate 5 tracks (on average)
                     # Vary the number according to Poisson distribution
                     # Generate pi+, pi-, e+ and e-
                     # with a normal distributed transversal momentum
                     # with a center of 5 GeV and a width of 1 GeV
                     # a normal distributed phi angle,
                     # center of 180 degree and a width of 30 degree
                     # Generate theta angles uniform in cos theta
                     # between 17 and 150 degree
                     # normal distributed vertex generation
                     # around the origin with a sigma of 2cm in the xy plane
                     # and no deviation in z
                     # all tracks sharing the same vertex per event
    'nTracks': 1,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normal',
    'momentumParams': [2, 0.2],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [30, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0.0, 0.5],
    'yVertexParams': [0.0, 0.5],
    'zVertexParams': [0.0, 0.5],
    'independentVertices': False,
    })

# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

# svddigi.param('statisticsFilename', 'digi.root')
svddigi.param('ElectronicEffects', True)

# create processing path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(svddigi)
main.add_module(svdclust)
main.add_module(analyze)
# main.add_module(output)

# generate events
process(main)

# show call statistics
print(statistics)
