#!/usr/bin/env python
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

# EvtMetaGen - generate event meta data
evtmetagen = register_module('EvtMetaGen')
evtruninfo = {'ExpList': [0], 'RunList': [0], 'EvtNumList': [1]}
evtmetagen.param(evtruninfo)

# Geometry parameter loader
paramloader = register_module('ParamLoaderXML')
paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

# Geometry builder
geobuilder = register_module('GeoBuilder')

# Overlap Checker
overlapchecker = register_module('OverlapChecker')
overlapchecker.param('Tolerance', 0.01)

# Saves the geometry as a Root file
geosaver = register_module('GeoSaver')
geosaver.param('Filename', 'Belle2Geo.root')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtmetagen)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(overlapchecker)
main.add_module(geosaver)

# Process one event
process(main)
