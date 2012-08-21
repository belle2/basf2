#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Opens the output of the MCFittingEvtGen.py example steering file and
# visualises GFTracks and PXD/SVD/CDC simhits using the TrackingDisplay
# module.
# The input file can also be set using
#  basf2 -i MyInputFile.root tracking/examples/trackingDisplay_simpleinput.py
#

import os
import random
from basf2 import *

# create paths
main = create_path()

input = register_module('SimpleInput')
# set the input file, in this case, the output of MCFittingEvtGen.py example
input.param('inputFileName', 'MCFittingEvtGenOutput.root')

main.add_module(input)

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

main.add_module(gearbox)
main.add_module(geometry)

display = register_module('TrackingDisplay')
# The Options parameter is a combination of:
# A autoscale errors - use when hits are too small to be seen
#   (because of tiny errors)
# D draw detectors - draw simple detector representation (with different size)
#   for each hit
# H draw track hits
# R draw raw simhits (not associated with a track) - drawn as points only
# G draw geometry (whole detector)
# M draw track markers - intersections of track with detector planes
#   (use with T)
# P draw detector planes
# S scale manually - spacepoint hits are drawn as spheres and scaled with
#   errors
# T draw track (straight line between detector planes)
# X silent - open TEve, but don't actually draw anything
#
# Note that you can always turn off an individual detector component or track
# interactively by removing its checkmark in the 'Eve' tab.
display.param('Options', 'MHTGR')  # default

# show all SimHits (default)
display.param('AddPXDHits', True)
display.param('AddSVDHits', True)
display.param('AddCDCHits', True)

main.add_module(display)

process(main)
print statistics
