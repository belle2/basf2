#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Starts ParticleGun simulation and shows MCParticles,
# SimHits and  GFTracks using the Display module.
#
# To view testbeam geometry in the EventDisplay window, go to Eve tab, and in
# Scenes/Geometry pane check "Top" and uncheck "Minimal geometry extract".

# The input file can also be set using
#  basf2 display/example/display.py -i MyInputFile.root
#

from basf2 import *

set_log_level(LogLevel.ERROR)

# ParticleGun
particlegun = register_module('ParticleGun')

# number of primaries per event
particlegun.param('nTracks', 1)

# DESY electrons:
particlegun.param('pdgCodes', [11])
# momentum magnitude 2 GeV/c or something above or around.
# At DESY we can 6 GeV/c(+-5%) electron beam.
# Beam divergence 2mrad not covered yet (we need some starting point location)
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [1.0, 0.0])
# momentum direction
particlegun.param('thetaGeneration', 'fixed')
particlegun.param('thetaParams', [90.0, 0.0])
particlegun.param('phiGeneration', 'fixed')
particlegun.param('phiParams', [0.0, 0.0])
# gun displacement
particlegun.param('vertexGeneration', 'fixed')
# Set xVertexParams to [-30.0,0.0] to move the gun inside the magnet
particlegun.param('xVertexParams', [-90.0, 0.0])
particlegun.param('yVertexParams', [0.0, 0.0])
particlegun.param('zVertexParams', [0.0, 0.0])
particlegun.param('independentVertices', True)

# Create Event information
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'EvtNumList': [100], 'RunList': [1]})


# create geometry
gearbox = register_module('Gearbox')
gearbox.param('Filename', 'testbeam/vxd/VXD.xml')

geometry = register_module('Geometry')
# Since Geometry is only required for track extrapolation in inner detectors,
# we'll exclude EKLM and ECL (saves about 20s in startup time)
geometry.param('Components', ['MagneticField', 'TB'])

# Run simulation
simulation = register_module('FullSim')

# PXD/SVD digitizer
PXDDigi = register_module('PXDDigitizer')
SVDDigi = register_module('SVDDigitizer')
# PXD/SVD clusterizer
PXDClust = register_module('PXDClusterizer')
SVDClust = register_module('SVDClusterizer')

display = register_module('Display')

# The Options parameter is a combination of:
# A autoscale PXD/SVD errors - use when hits are too small to be seen
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
display.param('Options', 'AHTM')  # default

# should hits always be assigned to a particle with c_PrimaryParticle flag?
# with this option off, many tracking hits will be assigned to secondary e-
display.param('AssignHitsToPrimaries', 0)

# show all primary MCParticles?
display.param('ShowAllPrimaries', True)

# show all charged MCParticles? (SLOW)
display.param('ShowCharged', False)

# show tracks?
display.param('ShowGFTracks', True)

# save events non-interactively (without showing window)?
display.param('Automatic', False)


# create paths
main = create_path()
main.add_module(evtmetagen)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(PXDDigi)
main.add_module(SVDDigi)
main.add_module(PXDClust)
main.add_module(SVDClust)
main.add_module(display)

process(main)
