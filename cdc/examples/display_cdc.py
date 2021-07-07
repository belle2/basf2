#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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

import basf2 as b2

# create paths
main = b2.create_path()

rootinput = b2.register_module('RootInput')
# rootinput = register_module('SeqRootInput')
# no input file set, use -i

# create geometry
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
# Since Geometry is only required for track extrapolation in inner detectors,
# we'll exclude ECL (saves about 10s in startup time)
# geometry.param('excludedComponents', ['ECL','CDC','PXD','SVD','EKLM','ARICH','BKLM'])
geometry.param('components', ['TOP'])

main.add_module(rootinput)
main.add_module(gearbox)
main.add_module(geometry)

display = b2.register_module('Display')

# The options parameter is a combination of:
# D draw detectors - draw simple detector representation (with different size)
#   for each hit
# H draw track hits
# M draw track markers - intersections of track with detector planes
# P draw detector planes
# S scale manually - spacepoint hits are drawn as spheres and scaled with
#   errors
#
# Note that you can always turn off an individual detector component or track
# interactively by removing its checkmark in the 'Eve' tab.
#
# only makes sense when showTrackLevelObjects/showTrackCandidates is used
display.param('options', 'MH')  # default

# should hits always be assigned to a particle with c_PrimaryParticle flag?
display.param('assignHitsToPrimaries', False)
display.param('showMCInfo', False)

# show all primary MCParticles?
# display.param('showAllPrimaries', True)

# show all charged MCParticles? (SLOW)
# display.param('showCharged', True)

# show all neutral MCParticles? (SLOW)
# display.param('showNeutrals', True)

# show tracks, vertices, eclgammas?
display.param('showTrackLevelObjects', True)

# show track candidates?
# You most likely don't want this unless you are a tracking developer
display.param('showTrackCandidates', True)
# If showTrackCandidates is true, you can set this option to switch between
# PXD/SVDClusters and PXD/SVDTrueHits
# display.param('useClusters', True)
display.param('useClusters', False)

# save events non-interactively (without showing window)?
display.param('automatic', False)

# change to True to show the full TGeo geometry instead of simplified extract
# display.param('fullGeometry', True)
display.param('fullGeometry', False)
# Show CDCHits.
display.param('showCDCHits', True)

main.add_module(display)

b2.process(main)
# print statistics(statistics.INIT)
