#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

# register necessary modules
evtmetagen = register_module('EvtMetaGen')

# generate one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [5])

evtmetainfo = register_module('EvtMetaInfo')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')

evtgeninput.param('boost2LAB', True)

# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# create paths
main = create_path()

# add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)

cdcdigi = register_module('CDCDigitizer')
main.add_module(cdcdigi)

pxd_digi = register_module('PXDDigitizer')
main.add_module(pxd_digi)

main.add_module(register_module('PXDClusterizer'))

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.param('UsePXDHits', True)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UseCDCHits', True)
main.add_module(mctrackfinder)

genfit = register_module('GenFitter')
main.add_module(genfit)

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
