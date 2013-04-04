#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

logging.log_level = LogLevel.WARNING

# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# PXD digitization module
pxddigi = register_module('PXDDigitizer')
# PXD clustering module
pxdclust = register_module('PXDClusterizer')
# RootOutput
output = register_module('RootOutput')
output.param('outputFileName', 'HitErrorGenOutput.root')

# Specify number of events to generate
evtmetagen.param({'EvtNumList': [10], 'RunList': [1]})

# Set parameters for particlegun
particlegun.param({  # Generate 5 tracks on average
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
    'momentumParams': [5, 0.2],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'thetaGeneration': 'normal',
    'thetaParams': [0.0, 0.01],
    'vertexGeneration': 'normal',
    'xVertexParams': [0.0, 0.01],
    'yVertexParams': [0, 0.01],
    'zVertexParams': [-10, 0.01],
    'independentVertices': True,
    })

# Select subdetectors to be built
# geometry.param('Components', ['PXD'])

gearbox.param('Filename', 'pxd/testbeam/SimpleDEPFETModule_Belle2.xml')

# pxddigi.param('statisticsFilename', 'digi.root')
pxddigi.param('ElectronicEffects', True)
pxddigi.param('SimpleDriftModel', False)

# create processing path
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(pxddigi)
main.add_module(pxdclust)
main.add_module(output)

# generate events
process(main)

# show call statistics
print statistics
