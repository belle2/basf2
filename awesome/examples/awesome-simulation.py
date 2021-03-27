#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Don't show all the messages :)
set_log_level(LogLevel.ERROR)

# Register modules

# lets generate empty events
evtmetagen = register_module('EventInfoSetter')
# and shoot some particles into them
particlegun = register_module('ParticleGun')
# load the simulation parameters
gearbox = register_module('Gearbox')
# build the awesome geometry
geometry = register_module('Geometry')
# simulate our awesome detector
simulation = register_module('FullSim')
# do something with the awesome data
analysis = register_module('Awesome')
# save the awesome results
output = register_module('RootOutput')
# an show some progress of the awesome simulation
progress = register_module('Progress')


# Set the parameters for the particle gun
particlegun.param({
    # Shoot electrons and positrons
    'pdgCodes': [11, -11],
    # 5 particles per event
    'nTracks': 5,
    # but let the number be poisson distributed
    'varyNTracks': True,
    # with a fixed momentum
    'momentumGeneration': 'fixed',
    # of 7 GeV
    'momentumParams': [7.0],
    # and a gaussian distributed theta angle
    'thetaGeneration': 'normal',
    # with mean 0 degree and width 1 degree
    'thetaParams': [0.0, 1.0],
    # and a uniform distributed phi angle
    'phiGeneration': 'uniform',
    # between 0 and 360 degree
    'phiParams': [0, 360.0],
    # but from a fixed position
    'vertexGeneration': 'fixed',
    # namely 0,0,0
    'xVertexParams': [0.0],
    'yVertexParams': [0.0],
    'zVertexParams': [0.0],
    # and the same vertex vor all particles
    'independentVertices': False,
})

# Main XML parameter file to load, relative to global data directory
gearbox.param('fileName', 'awesome/detector.xml')

# Lets see some more information on geometry building
geometry.set_log_level(LogLevel.INFO)

# and also on our own module
analysis.set_log_level(LogLevel.INFO)

# And write the results to awesome-simulation.root
output.param('outputFileName', 'awesome-simulation.root')

# Here we create a processing path and add the modules
main = create_path()
main.add_module(evtmetagen)
main.add_module(gearbox)
main.add_module('Geometry', useDB=False)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(analysis)
main.add_module(output)
main.add_module(progress)

# Now lets do the processing of the awesome events
process(main)

# Print call statistics of our awesome event processing
print(statistics)
