#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
set_log_level(LogLevel.WARNING)

# Register necessary modules:
# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
eventinfo = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# CDC digitization module
cdcdigi = register_module('CDCDigitizer')

# ============================================================================
# Set a fixed random seed for particle generation:
set_random_seed(3038402)

# Set the number of events to be generate and processed (xxxx events)
eventinfo.param({'evtNumList': [1000], 'runList': [1]})

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
    'varyNTracks': False,
    'pdgCodes': [211, -211],
    'momentumGeneration': 'uniform',
    'momentumParams': [0.2, 3.0],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniformCosinus',
    'thetaParams': [15, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0.0, 0.1],
    'yVertexParams': [0.0, 0.1],
    'zVertexParams': [0.0, 0.1],
    'independentVertices': False,
    })

# ============================================================================
# Print the parameters of the particle gun
print_params(particlegun)

# Select subdetectors to be built
# geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

# ============================================================================
# create processing path
main = create_path()
main.add_module(eventinfo)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(cdcdigi)
# Root output
simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', '../CDCOutputGen.root')
main.add_module(simpleoutput)
# generate events
process(main)

# show call statistics
print statistics
