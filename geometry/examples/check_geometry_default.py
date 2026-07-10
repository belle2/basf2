#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This steering file creates the default Belle II
# detector geometry from the conditions database
# (nominal geometry, e.g. as in release-10) and
# checks for overlaps
######################################################

import basf2 as b2
from sys import argv

# Write all log messages (including the overlaps found by OverlapChecker)
# also to a file, in addition to the console
b2.logging.add_file('check_geometry_default.log')

# To pin the geometry to a specific release, override the global tag, e.g.:
# b2.conditions.override_globaltag("release-10-00-00")

# Create main path
main = b2.Path()
# Add modules to main path
# EventInfoSetter defaults to exp 0, run 0, which corresponds to the
# nominal Belle II geometry
main.add_module("EventInfoSetter")
# Geometry builder
# useDB=True (the default) loads the geometry configuration from the
# conditions database, so no Gearbox/local XML files are needed and the
# full default detector is built without exclusions
main.add_module("Geometry", useDB=True)
# Overlap checker
if len(argv) > 1:
    main.add_module("OverlapChecker", points=int(argv[1]))
else:
    main.add_module("OverlapChecker")
# Save overlaps to file to be able to view them with b2display
main.add_module("RootOutput", outputFileName="Overlaps_default.root")
# Process one event
b2.process(main)
