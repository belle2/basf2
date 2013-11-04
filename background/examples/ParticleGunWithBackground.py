#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# This steering file shows how to comibine a particle gun simulated data
# with background using the background mixer.
#
# Example steering file - 2012 Belle II Collaboration
##############################################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')

# ============================================================================
# Setting the random seed for particle generation
set_random_seed(1028307)

# ============================================================================
# Set the list of particle codes (PDG codes) for the particles to be generated.
particlegun.param('pdgCodes', [-211, 211])

# ============================================================================
# Number of tracks to be generated per event:
particlegun.param('nTracks', 1)

# ============================================================================
# Vary the number of track
particlegun.param('varyNTracks', False)

# ============================================================================
# Set the parameters for the random generation of particle momentum:
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [1.0])
#
# ============================================================================
# Set the parameters for generation of the particle polar angle:
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [90, 5])
#
# ============================================================================
# Set the parameters for generation of the particle azimuth
# angle:
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [17, 150])

# ============================================================================
# Set the parameters for generation of the event vertex
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [0, 0.7])
particlegun.param('yVertexParams', [0, 0.7])
particlegun.param('zVertexParams', [0, 1.0])

# ============================================================================
# Set independent vertices for each particle
particlegun.param('independentVertices', True)

# ============================================================================
# Create the necessary modules to perform a simulation
#
# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [60], 'runList': [1]})
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])
# Run simulation
simulation = register_module('FullSim')
# Mix some background to simulation data
bgmixer = register_module('MixBkg')
bgmixer.param('BackgroundFiles', ['rof*.root'])
bgmixer.param('AnalysisMode', False)
bgmixer.set_log_level(LogLevel.INFO)
# Simulate SVD response
svddigi = register_module('SVDDigitizer')
svddigi.param('PoissonSmearing', True)
svddigi.param('ElectronicEffects', True)

# Analyze SVD response
svdclust = register_module('SVDClusterizer')
# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'ParticleGunWithBackgroundOutput.root')

# ============================================================================
# Construct processing path

main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(bgmixer)
main.add_module(svddigi)
main.add_module(svdclust)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
