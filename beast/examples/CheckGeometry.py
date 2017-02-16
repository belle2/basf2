#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file creates the Belle II detector
# geometry and checks for overlaps
######################################################

from basf2 import *

evtmeta = register_module('EventInfoSetter')

# Geometry parameter loader
gearbox = register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase2.xml')

# Geometry builder
geometry = register_module('Geometry')

# Overlap checker
overlapchecker = register_module('OverlapChecker')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtmeta)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(overlapchecker)

# Process one event
process(main)
