#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is validation steering file for the PXD of Belle2 simulation.
# Create base on ECLMuon.C validation and PXDTest.py
#  PXDValidationGen.py, PXDValidationTTree, PXDValidationTTreeSimHit
#                       PXDValidationTTreeDigit and PXDValidation3.C
#    ROOT macro for validation      ver. 0.1
#                     06 November 2013
#                     Peter Kodys (Charles Univ. Prague)
#                     peter.kodys@mff.cuni.cz
#
#  History of update
#  06 Nov 2013    ver. 0.1   First example
#
##############################################################################

from basf2 import *
from PXDValidationTTree import PXDValidationTTree
from PXDValidationTTreeSimHit import PXDValidationTTreeSimHit
from PXDValidationTTreeDigit import PXDValidationTTreeDigit
# set_log_level(LogLevel.ERROR)
# logging.log_level = LogLevel.WARNING
set_log_level(LogLevel.WARNING)

# Register necessary modules:
# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Create geometry:
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
# SVD digitization module
# svddigi = register_module('SVDDigitizer')
# SVD clusterizer
# svdclust = register_module('SVDClusterizer')
# RootOutput
output = register_module('RootOutput')

# ============================================================================
# Set a fixed random seed for particle generation:
set_random_seed(3038402)

analyze = PXDValidationTTree()
analyzeSimHit = PXDValidationTTreeSimHit()
analyzeDigit = PXDValidationTTreeDigit()

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
# particlegun.param('pdgCodes', [-11, 11])

# ============================================================================
# Setting the number of tracks to be generated per event:
# particlegun.param('nTracks', 1)

# Set the number of events to be generate and processed (xxxx events)
evtmetagen.param({'evtNumList': [1000], 'runList': [1]})

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
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normal',
    'momentumParams': [2, 0.2],
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

# Set output filename
output.param('outputFileName', 'PXDValidationOutput3.root')

# Select subdetectors to be built
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

# pxddigi.param('statisticsFilename', 'VXDDiags.root')
pxddigi.param('statisticsFilename', 'PXDValidationDiags3.root')
# pxddigi.param('ElectronicEffects', True)
pxddigi.param('ElectronicEffects', False)
pxddigi.param('SimpleDriftModel', False)
pxddigi.param('PoissonSmearing', True)
pxddigi.param('NoiseSN', 1.0)

pxdclust.param('NoiseSN', 1.0)
pxdclust.param('TanLorentz', 0.0)

# svddigi.param('statisticsFilename', 'SVDDiags.root')
# svddigi.param('PoissonSmearing', True)
# svddigi.param('ElectronicEffects', True)
# svddigi.param('statisticsFilename', 'SVDTestDiags.root')
# svddigi.param('storeWaveforms', False)

# ============================================================================
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
# main.add_module(svddigi)
# main.add_module(svdclust)
main.add_module(analyze)
main.add_module(analyzeSimHit)
main.add_module(analyzeDigit)
# main.add_module(output)

# generate events
process(main)

# show call statistics
print statistics
