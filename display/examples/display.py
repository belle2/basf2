#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Opens the MCFittingEvtGenOutput.root file and shows MCParticles,
# SimHits and  GFTracks using the Display module.

# The input file can also be set using
#  basf2 display/example/display.py -i MyInputFile.root
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

display = register_module('Display')

# The Options parameter is a combination of:
# A autoscale errors - use when hits are too small to be seen
#   (because of tiny errors)
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
display.param('Options', 'AHTGM')  # default

# should hits always be assigned to a particle with c_PrimaryParticle flag?
# with this option off, many tracking hits will be assigned to secondary e-
display.param('AssignHitsToPrimaries', 1)

# show all primary MCParticles?
display.param('ShowAllPrimaries', True)

# show all charged MCParticles? (SLOW)
# display.param('ShowCharged', True)

# show tracks?
display.param('ShowGFTracks', True)

main.add_module(display)

process(main)
print statistics
