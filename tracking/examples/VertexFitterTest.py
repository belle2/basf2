#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *

# Register necessary modules
evtmetagen = register_module('EvtMetaGen')
evtmetainfo = register_module('EvtMetaInfo')

# Create geometry

# Geometry parameter loader
gearbox = register_module('Gearbox')

# Geometry builder
geometry = register_module('Geometry')

geometry.param('Components', ['MagneticField', 'CDC'])
# geometry.logging.log_level = LogLevel.DEBUG

# Simulation
pGun = register_module('ParticleGun')

g4sim = register_module('FullSim')
# g4sim.logging.log_level = LogLevel.DEBUG

mcparticle = register_module('PrintMCParticles')

# Digitizer
cdcDigitizer = register_module('CDCDigitizer')
# pxdDigitizer = register_module('PXDDigitizer')
# pxdClusterizer = register_module('PXDClustering')

# Find MCTracks
mctrackfinder = register_module('MCTrackFinder')
# mctrackfinder.logging.log_level = LogLevel.DEBUG

# Fitting
cdcfitting = register_module('GenFitter')
# cdcfitting.logging.log_level = LogLevel.DEBUG

# Register VertexFitter
vertexfitter2 = register_module('VertexFitter2')
# rave.logging.log_level = LogLevel.INFO

# Register VertexFitterStat
# vertexfitterstat = register_module('VertexFitterStatistics')
# vertexfitterstat.logging.log_level = LogLevel.INFO

# print
printcol = register_module('PrintCollections')

# Output
output = register_module('SimpleOutput')

# Set parameters

# one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [20])

param_pGun = {
    'pdgCodes': [13, -13],
    'nTracks': 3,
    'momentumGeneration': 'uniform',
    'momentumParams': [1., 1.],
    'thetaGeneration': 'fixed',
    'thetaParams': [110., 100.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }

pGun.param(param_pGun)
mcparticle.param('onlyPrimaries', 1)

param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

param_mctrackfinder = {'UseCDCHits': 1, 'UseSVDHits': 0, 'UsePXDHits': 0}
#    'Smearing': 0,
mctrackfinder.param(param_mctrackfinder)

param_cdcfitting = {'mcTracks': 1, 'FilterId': 0}
#    'NIterations': 5,
#    'ProbCut': 0.001,

cdcfitting.param(param_cdcfitting)

# VertexFitter2 Parameter
vertexfitter2.param('resultFilePath', '/home/polyun/Belle2/results/test')
vertexfitter2.param('steeringFile', os.path.abspath(sys.argv[0]))

# Name of the output root file
output.param('outputFileName', 'TEST.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(mcparticle)

main.add_module(printcol)
main.add_module(cdcDigitizer)

main.add_module(mctrackfinder)
main.add_module(cdcfitting)
main.add_module(vertexfitter2)
# main.add_module(vertexfitterstat)
main.add_module(output)

# Process events
process(main)

print statistics
