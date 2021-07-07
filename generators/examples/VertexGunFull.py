#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import set_log_level, LogLevel, register_module, create_path, process, statistics, set_random_seed, print_params

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
set_random_seed(1028307)

# first vertex
vertex1 = register_module('ParticleGun')

# setting the number of tracks and pdg codes for the first vertex
vertex1.param('nTracks', -1)  # a negative track number means one track for every PDG code
vertex1.param('pdgCodes', [-11, 11, -13, 13])

# set the starting parameters of the particle originating from the first vertex
vertex1.param('momentumGeneration', 'normal')
vertex1.param('momentumParams', [1, 0.05])

vertex1.param('thetaGeneration', 'uniform')
vertex1.param('thetaParams', [17, 150])

vertex1.param('phiGeneration', 'uniform')
vertex1.param('phiParams', [0, 360])

# make sure all particles of vertex1 in one event start at the same postion
vertex1.param('independentVertices', False)

vertex1.param('vertexGeneration', 'fixed')
vertex1.param('xVertexParams', [0.0, 0.0])
vertex1.param('yVertexParams', [0.0, 0.0])
vertex1.param('zVertexParams', [0.0, 0.0])

# second vertex
vertex2 = register_module('ParticleGun')

# setting the number of tracks and pdg codes for the first vertex
vertex2.param('nTracks', -1)  # a negative track number means one track for every PDG code
vertex2.param('pdgCodes', [-211, 211, -211])

# set the starting parameters of the particle originating from the first vertex
vertex2.param('momentumGeneration', 'normal')
vertex2.param('momentumParams', [1, 0.05])

vertex2.param('thetaGeneration', 'uniform')
vertex2.param('thetaParams', [17, 150])

vertex2.param('phiGeneration', 'uniform')
vertex2.param('phiParams', [0, 360])

# make sure all particles of vertex2 in one event start at the same postion
vertex2.param('independentVertices', False)

vertex2.param('vertexGeneration', 'fixed')
vertex2.param('xVertexParams', [1, 1])
vertex2.param('yVertexParams', [1, 1])
vertex2.param('zVertexParams', [1, 1])

# third vertex
vertex3 = register_module('ParticleGun')

# setting the number of tracks and pdg codes for the first vertex
vertex3.param('nTracks', -1)  # a negative track number means one track for every PDG code
vertex3.param('pdgCodes', [-11, 11])

# set the starting parameters of the particle originating from the first vertex
vertex3.param('momentumGeneration', 'normal')
vertex3.param('momentumParams', [0.5, 0.01])

vertex3.param('thetaGeneration', 'uniform')
vertex3.param('thetaParams', [60, 120])

vertex3.param('phiGeneration', 'uniform')
vertex3.param('phiParams', [0, 360])

# make sure all particles of vertex3 in one event start at the same postion
vertex3.param('independentVertices', False)

vertex3.param('vertexGeneration', 'fixed')
vertex3.param('xVertexParams', [0, 0])
vertex3.param('yVertexParams', [0, 0])
vertex3.param('zVertexParams', [3, 3])

# ============================================================================
# Print the parameters of the particle guns
print_params(vertex1)
print_params(vertex2)
print_params(vertex3)

# ============================================================================
# Now lets create the necessary modules to perform a simulation
#
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
# Save output of simulation
output = register_module('RootOutput')

# Setting the option for all non particle gun modules: want to process 100 MC
# events
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})

# Set output filename
output.param('outputFileName', 'ParticleGunOutput.root')

# ============================================================================
# Do the simulation
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.logging.log_level = LogLevel.INFO
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(vertex1)
main.add_module(vertex2)
main.add_module(vertex3)
main.add_module(mcparticleprinter)
main.add_module(simulation)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
