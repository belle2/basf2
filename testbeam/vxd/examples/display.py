#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will simulate several testbeam events, reconstruct
# and fit tracks, and display each event.
import os
from basf2 import *
from subprocess import call

set_log_level(LogLevel.ERROR)

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('expList', [0])
evtmetagen.param('runList', [1])
evtmetagen.param('evtNumList', [100])
evtmetainfo = register_module('EvtMetaInfo')

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

display = register_module('Display')

# The Options parameter is a combination of:
# D draw detectors - draw simple detector representation (with different size)
#   for each hit
# H draw track hits
# M draw track markers - intersections of track with detector planes
#   (use with T)
# P draw detector planes
# S scale manually - spacepoint hits are drawn as spheres and scaled with
#   errors
# T draw track (straight line between detector planes)
#
# Note that you can always turn off an individual detector component or track
# interactively by removing its checkmark in the 'Eve' tab.
#
# This option only makes sense when ShowGFTracks is true
display.param('options', 'HTM')  # default

# should hits always be assigned to a particle with c_PrimaryParticle flag?
# with this option off, many tracking hits will be assigned to secondary e-
display.param('assignHitsToPrimaries', 0)

# show all primary MCParticles?
display.param('showAllPrimaries', True)

# show all charged MCParticles? (SLOW)
display.param('showCharged', False)

# show tracks?
display.param('showGFTracks', True)

# save events non-interactively (without showing window)?
display.param('automatic', False)

# Use clusters to display tracks
display.param('useClusters', True)

# Display the testbeam geometry rather than Belle II extract
display.param('fullGeometry', True)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
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
main.add_module(display)

# Process events
process(main)

print statistics
