#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file creates the Belle II detector
# geometry at phase 2
######################################################

from basf2 import *

evtmeta = register_module('EventInfoSetter')

# Geometry parameter loader
gearbox = register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase2.xml')

# Geometry builder
geometry = register_module('Geometry')
geometry.param('excludedComponents', ['ECL'])

# Overlap checker
overlapchecker = register_module('OverlapChecker')

# Display geometry
display = register_module('Display')
display.param('fullGeometry', True)

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtmeta)
main.add_module(gearbox)
main.add_module(geometry)
# main.add_module(overlapchecker)
main.add_module(display)

# Process one event
process(main)
