#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This steering file creates the Belle II detector
# geometry and checks for overlaps
######################################################

import basf2 as b2
from sys import argv

# Create main path
main = b2.Path()
# Add modules to main path
main.add_module("EventInfoSetter")
# Geometry parameter loader
gearbox = b2.register_module('Gearbox')
gearbox.param('fileName', 'geometry/Belle2.xml')
main.add_module(gearbox)
# Geometry builder
# useDB=False builds the geometry from the local XML files instead of the
# conditions database, so excludedComponents takes effect and locally added
# parameters (e.g. new BeamPipeGeo entries) are picked up
main.add_module("Geometry", excludedComponents=['PXD', 'HeavyMetalShield', 'SVD'], useDB=False)
# Overlap checker
if len(argv) > 1:
    main.add_module("OverlapChecker", points=int(argv[1]))
else:
    main.add_module("OverlapChecker")
# Save overlaps to file to be able to view them with b2display
main.add_module("RootOutput", outputFileName="Overlaps.root")
# Process one event
b2.process(main)
