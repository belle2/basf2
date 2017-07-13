#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from tracking import add_tracking_reconstruction
import sys

# --------------------------------------------------------------------
# Example of using TOP reconstruction
# needs reconstructed tracks (Tracks), extrapolated to TOP (ExtHits)
# log likelihoods in TOPLikelihoods
# relation from Tracks to TOPLikelihoods
# --------------------------------------------------------------------

# movie for 10 seconds (24 images / sec)
istep = int(sys.argv[1])
# divide the theta range into 240 bins
iTheta = 32 + (122 - 32) / 240.0 * istep
# divide the momentum range into 240 bins
iMomentum = 1 + (5 - 1) / 240.0 * istep
pdgCode = int(sys.argv[2])
# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

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
particlegun.param('pdgCodes', [pdgCode])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [iMomentum, iMomentum])
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [85, 85])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 0])
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
svd_digitizer = register_module('SVDDigitizer')
main.add_module(svd_digitizer)
svd_clusterizer = register_module('SVDClusterizer')
main.add_module(svd_clusterizer)

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
topreco = register_module('TOPReconstructorPDF')
topreco.logging.log_level = LogLevel.DEBUG  # remove or comment to suppress printout
topreco.logging.debug_level = 2  # or set level to 0 to suppress printout
topreco.param("writeNPdfs", 1)
main.add_module(topreco)

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'pdf_momentum_scan_pid%d_step_%03d.root' % (pdgCode, istep))
branches = [
    'ExtHits',
    'TOPDigits',
    'TOPRawWaveforms',
    'TOPPDFCollections',
    'TOPSmallestPullCollections'
]
output.param('branchNames', branches)
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
