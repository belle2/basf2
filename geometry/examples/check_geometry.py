#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file creates the Belle II detector
# geometry and checks for overlaps
######################################################

from basf2 import Path, process

# Create main path
main = Path()
# Add modules to main path
main.add_module("EventInfoSetter")
# Geometry parameter loader
main.add_module("Gearbox")
# Geometry builder
main.add_module("Geometry")
# Overlap checker
main.add_module("OverlapChecker")
# Save overlaps to file to be able to view them with b2display
main.add_module("RootOutput", outputFileName="Overlaps.root")
# Process one event
process(main)
