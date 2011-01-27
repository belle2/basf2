#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# This steering file creates the Belle II detector
# geometry, does an overlap check and saves it as
# a Root file.
#
# Example steering file - 2011 Belle II Collaboration
######################################################

import os
from basf2 import *

# Register modules
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')
overlapchecker = register_module('OverlapChecker')
geosaver = register_module('GeoSaver')

# Set parameters
paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))
geosaver.param('Filename', 'Belle2Geo.root')
overlapchecker.param('Tolerance', 0.01)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(overlapchecker)
main.add_module(geosaver)

# Process one event
process(main, 1, 1)
