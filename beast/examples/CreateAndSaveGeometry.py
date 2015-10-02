#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file creates the Belle II detector
# geometry, does an overlap check and saves it as
# a Root file.
#
# The tolerance for overlaps, which are considered
# to be problematic, is set to 1mm.
#
# Example steering file - 2011 Belle II Collaboration
######################################################

import os
from basf2 import *

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
# eventinfosetter.param({'evtNumList': [nevent], 'runList': [1], 'expList': [1]})

# Geometry parameter loader
gearbox = register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase2.xml')
# Geometry builder
geometry = register_module('Geometry')

# Overlap Checker
# overlapchecker = register_module('OverlapChecker')
# overlapchecker.param('Tolerance', 0.01)

# Saves the geometry as a Root file
geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'Belle2Geo.root')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
# main.add_module(overlapchecker)
main.add_module(geosaver)

# Process one event
process(main)
