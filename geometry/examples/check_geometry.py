#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

from basf2 import Path, process
from sys import argv

# Create main path
main = Path()
# Add modules to main path
main.add_module("EventInfoSetter")
# Geometry parameter loader
main.add_module("Gearbox")
# Geometry builder
main.add_module("Geometry")
# Overlap checker
if len(argv) > 1:
    main.add_module("OverlapChecker", points=int(argv[1]))
else:
    main.add_module("OverlapChecker")
# Save overlaps to file to be able to view them with b2display
main.add_module("RootOutput", outputFileName="Overlaps.root")
# Process one event
process(main)
