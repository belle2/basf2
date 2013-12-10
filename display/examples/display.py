#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Opens a .root file and shows MCParticles,
# SimHits and GFTracks using the Display module.
# Usage:
#  basf2 display/example/display.py -i MyInputFile.root
#
# Note: this file is also used by the 'b2display' command,
# so the following is also possible:
#  b2display MyInputFile.root
#
# If you want custom settings for b2display, you thus only need to
# edit this steering file.

from basf2 import *

# create paths
main = create_path()

rootinput = register_module('RootInput')
#no input file set, use -i

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
# Since Geometry is only required for track extrapolation in inner detectors,
# we'll exclude ECL (saves about 10s in startup time)
geometry.param('ExcludedComponents', ['ECL'])

main.add_module(rootinput)
main.add_module(gearbox)
main.add_module(geometry)

display = register_module('Display')

# The options parameter is a combination of:
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
# only makes sense when showTrackLevelObjects/showTrackCandidates is used
display.param('options', 'HTM')  # default

# should hits always be assigned to a particle with c_PrimaryParticle flag?
display.param('assignHitsToPrimaries', False)

# show all primary MCParticles?
display.param('showAllPrimaries', True)

# show all charged MCParticles? (SLOW)
# display.param('showCharged', True)

# show all neutral MCParticles? (SLOW)
# display.param('showNeutrals', True)

# show tracks, vertices, eclgammas?
display.param('showTrackLevelObjects', True)

# show track candidates?
# You most likely don't want this unless you are a tracking developer
display.param('showTrackCandidates', False)
# If showTrackCandidates is true, you can set this option to switch between
# PXD/SVDClusters and PXD/SVDTrueHits
display.param('useClusters', True)

# save events non-interactively (without showing window)?
display.param('automatic', False)

# change to True to show the full TGeo geometry instead of simplified extract
display.param('fullGeometry', False)

main.add_module(display)

process(main)
# print statistics(statistics.INIT)
