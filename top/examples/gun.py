#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from tracking import add_tracking_reconstruction
from simulation import add_svd_simulation

# --------------------------------------------------------------------
# Example of using TOP reconstruction
# needs reconstructed tracks (Tracks), extrapolated to TOP (ExtHits)
# log likelihoods in TOPLikelihoods
# relation from Tracks to TOPLikelihoods
# --------------------------------------------------------------------

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
main.add_module(eventinfosetter)

# Histogram manager immediately after master module
histo = b2.register_module('HistoManager')
histo.param('histoFileName', 'DQMhistograms.root')  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# Particle gun: generate multiple tracks
particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211, 321, -321])
particlegun.param('nTracks', 5)
particlegun.param('varyNTracks', True)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.5, 4])
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [32, 122])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
particlegun.param('independentVertices', False)
main.add_module(particlegun)

# Simulation
simulation = b2.register_module('FullSim')
main.add_module(simulation)

# PXD digitization & clustering
pxd_digitizer = b2.register_module('PXDDigitizer')
main.add_module(pxd_digitizer)
pxd_clusterizer = b2.register_module('PXDClusterizer')
main.add_module(pxd_clusterizer)

# SVD digitization & clustering
add_svd_simulation(main)

# CDC digitization
cdcDigitizer = b2.register_module('CDCDigitizer')
main.add_module(cdcDigitizer)

# TOP digitization
topdigi = b2.register_module('TOPDigitizer')
main.add_module(topdigi)

# tracking
add_tracking_reconstruction(main)

# Track extrapolation
ext = b2.register_module('Ext')
main.add_module(ext)

# Channel masker
main.add_module('TOPChannelMasker')

# TOP reconstruction
topreco = b2.register_module('TOPReconstructor')
topreco.logging.log_level = b2.LogLevel.DEBUG  # remove or comment to suppress printout
topreco.logging.debug_level = 2  # or set level to 0 to suppress printout
main.add_module(topreco)

# TOP DQM
topdqm = b2.register_module('TOPDQM')
main.add_module(topdqm)

# Output
output = b2.register_module('RootOutput')
output.param('outputFileName', 'TOPOutput.root')
main.add_module(output)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
