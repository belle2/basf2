#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *
from SVDHitErrors import SVDHitErrors

logging.log_level = LogLevel.ERROR

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
# PXD clustering module
svdclust = register_module('SVDClusterizer')

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({
    'nTracks': 1,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normal',
    'momentumParams': [2, 0.2],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'thetaGeneration': 'normal',
    'thetaParams': [30, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0.0, 1],
    'yVertexParams': [0.0, 1],
    'zVertexParams': [0.0, 1],
    'independentVertices': True,
    })

# Select subdetectors to be built
geometry.param('Components', ['SVD'])

# svddigi.param('statisticsFilename', 'digi.root')
svddigi.param('ElectronicEffects', True)
svdclust.param('TanLorentz_holes', 0.0)
svdclust.param('TanLorentz_electrons', 0.0)

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
main.add_module(SVDHitErrors())

# generate events
process(main)

# show call statistics
print statistics
