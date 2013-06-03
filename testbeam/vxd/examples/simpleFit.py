#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will simulate testbeam events, fit tracks and
# show tracking statistics
import os
from basf2 import *
from subprocess import call

set_log_level(LogLevel.ERROR)
set_random_seed(3)

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('expList', [0])
evtmetagen.param('runList', [1])
evtmetagen.param('evtNumList', [1000])

# Show progress of processing
progress = register_module('Progress')

gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName', 'testbeam/vxd/VXD.xml')

geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes this example much faster
geometry.param('Components', ['MagneticField', 'TB'])

# ParticleGun
particlegun = register_module('ParticleGun')

# number of primaries per event
particlegun.param('nTracks', 1)

# DESY electrons:
particlegun.param('pdgCodes', [-11])
# momentum magnitude 2 - 6 GeV/c
# Beam divergence 2mrad not covered yet (we need some starting point location)
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [2.0, 0.0])
# momentum direction
particlegun.param('thetaGeneration', 'fixed')
particlegun.param('thetaParams', [90.0, 0.0])
particlegun.param('phiGeneration', 'fixed')
particlegun.param('phiParams', [0.0, 0.0])
# gun displacement
particlegun.param('vertexGeneration', 'fixed')
# Set xVertexParams to [-30.0,0.0] to move the gun inside the magnet
particlegun.param('xVertexParams', [-90.0, 0.0])
particlegun.param('yVertexParams', [1.0, 0.0])
particlegun.param('zVertexParams', [0.0, 0.0])
particlegun.param('independentVertices', True)

g4sim = register_module('FullSim')
# this is needed for the MCTrackFinder to work correctly
g4sim.param('StoreAllSecondaries', True)
SVDDIGI = register_module('SVDDigitizer')
# SVDDIGI.logging.log_level = LogLevel.DEBUG
SVDDIGI.param('PoissonSmearing', True)
SVDDIGI.param('ElectronicEffects', True)

SVDCLUST = register_module('SVDClusterizer')
# SVDCLUST.logging.log_level = LogLevel.DEBUG

PXDDIGI = register_module('PXDDigitizer')
# PXDDIGI.logging.log_level = LogLevel.DEBUG
PXDDIGI.param('SimpleDriftModel', False)
PXDDIGI.param('PoissonSmearing', True)
PXDDIGI.param('ElectronicEffects', True)

PXDCLUST = register_module('PXDClusterizer')

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.logging.log_level = LogLevel.WARNING
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    }
mctrackfinder.param(param_mctrackfinder)

# mctrackfinder.logging.log_level = LogLevel.DEBUG

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('UseClusters', True)
trackfitchecker = register_module('TrackFitChecker')
# the results only show up at info or debug level
trackfitchecker.logging.log_level = LogLevel.INFO
# trackfitchecker.param('inspectTracks', True)
trackfitchecker.param('truthAvailable', True)
# trackfitchecker.param('testSi', True)
trackfitchecker.param('robustTests', True)
trackfitchecker.param('writeToRootFile', True)

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'TBSimulationOutput.root')

# Create paths
main = create_path()
# Add modules to paths
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(g4sim)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
main.add_module(SVDDIGI)
main.add_module(SVDCLUST)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
main.add_module(trackfitchecker)
main.add_module(output)

# Process events
process(main)

print statistics
