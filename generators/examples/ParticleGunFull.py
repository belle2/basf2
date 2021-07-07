#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import set_log_level, register_module, process, LogLevel, \
    set_random_seed, print_params, create_path, statistics
from simulation import add_simulation
from reconstruction import add_reconstruction

# suppress messages and warnings during processing:
set_log_level(LogLevel.WARNING)

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')

# ============================================================================
# Setting the random seed for particle generation
set_random_seed(123)

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
# the codes are given in an array, if only one code is used, the brackets
# should be kept:
# particlegun.param('pdgCodes', [11])

# there is no limit on how many codes can be given the particle gun will select
# randomly amongst the PDGcodes using a uniform distribution if nTracks>0,
# otherwise there will be one particle for each code in the list default is
# [-11, 11]
particlegun.param('pdgCodes', [-11, 11])

# ============================================================================
# Setting the number of tracks to be generated per event: this number can be
# any int>=0 default is 1
particlegun.param('nTracks', 10)

# a value o 0 means that a track should be created for each entry in the
# pdgCodes list, e.g. the following two lines would create two electrons and
# one pion per event.
# particlegun.param('pdgCodes', [11,11,211])
# particlegun.param('nTracks', 0)

# ============================================================================
# Varying the number of tracks per event can be achieved by setting varyNTacks
# to True. If so, the number of tracks will be randomized using possion
# distribution around the value of nTracks.  Only valid if nTracks>0, default
# is False
particlegun.param('varyNTracks', False)

# ============================================================================
# Each particle has a total momentum, a direction given as theta and phi and a
# vertex position in x, y and z. For each variable we need to specify the
# distribution and the parameters for the distribution. The number of
# parameters depends on the chosen distribution. All available distributions
# are listed here with required parameters are given in []. The distributions
# with suffix "Pt" can only be used for momentum generation and the ones with
# suffix "Cos" can only used for the generation of theta and phi.
#
# - fixed:       always use the exact same value [value]
# - uniform:     uniform distribution between min and max [min, max]
# - uniformPt:   uniform distribution of transverse momentum between a given
#                minimal and maximum value [min, max]
# - uniformCos:  uniform distribution of the cosine of the angle between min
#                and max of the absolute value [min, max]
# - normal:      normal (gaussian) distribution around mean with width of sigma
#                [mean, sigma]
# - normalPt:    normal distribution of transverse momentum [mean, sigma]
# - normalCos:   normal distribution of the cosine of the angle, not the
#                absolute value [mean, sigma]
# - inversePt:   generate momentum to obtain a flat distribution of the track
#                curvature (inverse transverse momentum) between a minimal and
#                maximal transverse momentum, [min_pt, max_pt]
# - polyline:    create the momentum to follow an arbitrary distribution given
#                as a list of x and y coordinates. All y coordinates must be
#                non-negative and at leas one y coordinate must be positive
#                [x1, x2, ..., xn, y1, y2, ..., yn]
# - polylinePt:  same as polyline but for the transverse momentum, not the
#                total momentum.
#                [x1, x2, ..., xn, y1, y2, ..., yn]
# - polylineCos: same as polyline but for the cosine of the angle, not the
#                absolute value
#                [x1, x2, ..., xn, y1, y2, ..., yn]
# - discrete:    a discrete spectrum consisting of possible values xi and their
#                weights wi (useful e.g. for radioactive sources)
#                [x1, x2, ..., xn, w1, w2, ..., wn]

# ============================================================================
# Momentum generation
#
# The default is a uniform momentum distribution between 0.05 and 3 GeV
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.05, 3])

# we could also generate a fixed momentum of 1 GeV
# particlegun.param('momentumGeneration', "fixed")
# particlegun.param('momentumParams', [1.0])

# or we could generate a normal distributed transverse momentum around 2 GeV
# with a width of 0.5 GeV
# particlegun.param('momentumGeneration', "normalPt")
# particlegun.param('momentumParams', [2.0, 0.5])

# to generate the momentum according to a cadmium 109
# source we could use
# particlegun.param('momentumGeneration', 'discreteSpectrum'),
# particlegun.param('momentumParams', [
#     22.1e-6, 25.0e-6, 88.0e-6,  # photon energies
#     82.6,    14.7,     3.65,    # weights
# ])

# ============================================================================
# polar angle, theta
# The default is a uniform theta distribution between 17 and 150 degree

particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [17, 150])

# We could also generate between 17 and 150 degrees with a flat distribution in
# cos(theta)
# particlegun.param('thetaGeneration', 'uniformCos')
# particlegun.param('thetaParams', [17, 150])

# or we could create a theta angle between 17 and 150 degree where the
# cos(theta) distribution is flat
# particlegun.param('thetaGeneration', "normal")
# particlegun.param('thetaParams', [90,5])

# Finally, we could use numpy to create events where the cos(theta)
# distribution follows a parabolic shape:
# import numpy as np
# # Create a set of 1000 x values spread uniformly over -1, 1
# x = np.linspace(-1,1,1000)
# # Create the corresponding distribution
# y = 1-x**2
# # Set the parameters by concatenating the x and y positions
# particlegun.param('thetaGeneration', "polylineCos")
# particlegun.param('thetaParams', list(x) + list(y))

# ============================================================================
# azimuth angle, phi
# The default is a uniform theta distribution between 0 and 360 degree

particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

# or we could create a normal distributed phi angle around 90 degrees with a
# width of 5 degrees
# particlegun.param('phiGeneration', "normal")
# particlegun.param('phiParams', [90,5])

# ============================================================================
# Vertex generation
# The default is a fixed vertex at (0,0,0) for all tracks
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])

# We could also generate a normal distributed vertex with mean at (0,0,0) and
# width of 10µm, 60nm and 190 µm in x, y and z respectively
# particlegun.param('vertexGeneration', 'fixed')
# particlegun.param('xVertexParams', [0, 10e-4])
# particlegun.param('yVertexParams', [0, 60e-7])
# particlegun.param('zVertexParams', [0, 190e-4])

# We can also specify a different distribution for any of the three axes, e.g.
# to make the z-vertex uniform between -10 and 10 cm and the xy vertex normal
# distributed around zero with a width of 0.1 cm we could use
# particlegun.param('vertexGeneration', 'normal')
# particlegun.param('xVertexParams', [0, 0.1])
# particlegun.param('yVertexParams', [0, 0.1])
# particlegun.param('zVertexGeneration', 'uniform')
# particlegun.param('zVertexParams', [-10, 10])

# ============================================================================
# Setting independent vertices for each particle The default is to create one
# event vertex for all particles per event. By setting independentVertices to
# True, a new vertex will be created for each particle default is False
particlegun.param('independentVertices', False)

# ============================================================================
# Print the parameters of the particle gun
print_params(particlegun)

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
# Save output of simulation
output = register_module('RootOutput')

# Setting the option for all non particle gun modules: want to process 100 MC
# events
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# Set output filename
output.param('outputFileName', 'ParticleGunOutput.root')

# ============================================================================
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)

main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
