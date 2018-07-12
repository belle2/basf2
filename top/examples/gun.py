#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from tracking import add_tracking_reconstruction
from simulation import add_svd_simulation

# --------------------------------------------------------------------
# Example of using TOP reconstruction
# needs reconstructed tracks (Tracks), extrapolated to TOP (ExtHits)
# log likelihoods in TOPLikelihoods
# relation from Tracks to TOPLikelihoods
# --------------------------------------------------------------------

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'DQMhistograms.root')  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
])
main.add_module(geometry)

# Particle gun: generate multiple tracks
particlegun = register_module('ParticleGun')
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
simulation = register_module('FullSim')
main.add_module(simulation)

# PXD digitization & clustering
pxd_digitizer = register_module('PXDDigitizer')
main.add_module(pxd_digitizer)
pxd_clusterizer = register_module('PXDClusterizer')
main.add_module(pxd_clusterizer)

# SVD digitization & clustering
add_svd_simulation(main)

# CDC digitization
cdcDigitizer = register_module('CDCDigitizer')
main.add_module(cdcDigitizer)

# TOP digitization
topdigi = register_module('TOPDigitizer')
main.add_module(topdigi)

# tracking
add_tracking_reconstruction(main)

# Track extrapolation
ext = register_module('Ext')
main.add_module(ext)

# TOP reconstruction
topreco = register_module('TOPReconstructor')
topreco.logging.log_level = LogLevel.DEBUG  # remove or comment to suppress printout
topreco.logging.debug_level = 2  # or set level to 0 to suppress printout
main.add_module(topreco)

# TOP DQM
topdqm = register_module('TOPDQM')
main.add_module(topdqm)

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'TOPOutput.root')
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
