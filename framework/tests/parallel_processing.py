#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Test the parallel processing functionality by simulating a few events
# using 2 processes (so nothing too taxing)

from basf2 import *

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
# the codes are given in an array, if only one code is used, the brackets
# should be kept: particlegun.param('pdgCodes',[11])
# there is no limit on how many codes can be given the particle gun will select
# randomly amongst the PDGcodes using a uniform distribution if nTracks>0,
# otherwise there will be one particle for each code in the list default is
# [-11, 11]
particlegun.param('pdgCodes', [211, -211, 321, -321])

# ============================================================================
# Setting the number of tracks to be generated per event:
# this number can be any int>=0
# default is 1
particlegun.param('nTracks', 1)

# a value o 0 means that a track should be created for each entry in the
# pdgCodes list, e.g. the following two lines would create two electrons and
# one pion per event.
# particlegun.param('pdgCodes', [11,11,211])
# particlegun.param('nTracks', 0)

# ============================================================================
# Varying the number of tracks per event can be achieved by setting varyNTacks
# to True. If so, the number of tracks will be randomized using possion
# distribution around the value of nTracks. Only valid if nTracks>0
# default is False
particlegun.param('varyNTracks', False)

# ============================================================================
# Setting the parameters for the random generation
# of particles momenta:
# Five different distributions can be used:
# - fixed:     always use the exact same value
# - uniform:   uniform distribution between min and max
# - uniformPt: uniform distribution of transverse momentum between min and max
# - normal:    normal distribution around mean with width of sigma
# - normalPt:  normal distribution of transverse momentum around mean with
#              width of sigma

# The default is a uniform momentum distribution between 0.05 and 3 GeV
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [3])

# we could also generate a fixed momentum of 1 GeV
# particlegun.param('momentumGeneration', "fixed")
# particlegun.param('momentumParams', [1.0])

# or we could generate a normal distributed transverse momentum around 2 GeV
# with a width of 0.5 GeV
# particlegun.param('momentumGeneration', "normalPt")
# particlegun.param('momentumParams', [2.0, 0.5])

# ============================================================================
# Setting the parameters for the random generation
# of the particle polar angle:
# Four different distributions can be used:
# - fixed:     always use the exact same value
# - uniform:   uniform distribution between min and max
# - uniformCosinus: uniform distribution of cos(theta) between min and max
# - normal:    normal distribution around mean with width of sigma
#
# The default is a uniform theta distribution between 17 and 150 degree
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [17, 90])

# or we could create a normal distributed theta angle around 90 degrees with a
# width of 5 degrees
# particlegun.param('thetaGeneration', "normal")
# particlegun.param('thetaParams', [90,5])

# or we could create a theta angle between 17 and 150 degree where the
# cos(theta) distribution is flat
# particlegun.param('thetaGeneration', "normal")
# particlegun.param('thetaParams', [90,5])

# ============================================================================
# Setting the parameters for the random generation
# of the particle azimuth angle:
# Three different distributions can be used:
# - fixed:     always use the exact same value
# - uniform:   uniform distribution between min and max
# - normal:    normal distribution around mean with width of sigma
#
# The default is a uniform theta distribution between 0 and 360 degree
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

# or we could create a normal distributed phi angle around 90 degrees with a
# width of 5 degrees
# particlegun.param('phiGeneration', "normal")
# particlegun.param('phiParams', [90,5])

# ============================================================================
# Setting the parameters for random generation of the event vertex
# Three different distributions can be used:
# - fixed:     always use the exact same value
# - uniform:   uniform distribution between min and max
# - normal:    normal distribution around mean with width of sigma
#
# The default is a normal distribution of the vertex
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])

# ============================================================================
# Setting independent vertices for each particle
# The default is to create one event vertex for all particles per event. By
# setting independentVertices to True, a new vertex will be created for each
# particle
# default is False
particlegun.param('independentVertices', False)

# ============================================================================
# Print the parameters of the particle gun
print_params(particlegun)

# ============================================================================
# Now lets create the necessary modules to perform a simulation

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
geometry.param('components', ['MagneticField', 'BeamPipe', 'PXD'])
# Run simulation
simulation = register_module('FullSim')
param_g4sim = {'RegisterOptics': 1, 'PhotonFraction': 0.3,
               'TrackingVerbosity': 0}
simulation.param(param_g4sim)

eventinfosetter.param({'evtNumList': [5], 'runList': [1]})

# Set output filename
output = register_module('RootOutput')
output.param('outputFileName', 'parallel_processing_test.root')

main = create_path()
# init path
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(particlegun)

# event path
main.add_module(geometry)
main.add_module(simulation)

# output path
main.add_module(output)
main.add_module(progress)
#main.add_module(register_module('PrintCollections'))

#test wether flags are what we expect
if particlegun.has_properties(ModulePropFlags.PARALLELPROCESSINGCERTIFIED):
    B2FATAL("ParticleGun has pp flag?")
if not simulation.has_properties(ModulePropFlags.PARALLELPROCESSINGCERTIFIED):
    B2FATAL("Simulation doesn't have pp flag?")

# Process events
set_nprocesses(2)
process(main)

# check wether output file contains correct number of events
from ROOT import TFile
from ROOT import TTree
file = TFile('parallel_processing_test.root')
tree = file.Get('tree')
if tree.GetEntries() != 5:
    B2FATAL('Created output file contains wrong number of events! (' + str(tree.GetEntries()) + ')')

nummcparticles = tree.Project("", "MCParticles.m_pdg")
if nummcparticles < 5:
    B2FATAL('Output file should contain at least five MCParticles!')

numhits = tree.Project("", "PXDSimHits.getArrayIndex()")
if numhits < 5:  # usually much more, existence is most important thing here
    B2FATAL('Output file should contain at least 5 hits!')


import os
os.remove('parallel_processing_test.root')
