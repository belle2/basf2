# Add the cut module to the pat#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from reconstruction import *
from modularAnalysis import *
from ROOT import Belle2

# Create path
main = create_path()

# Root input
roinput = register_module('RootInput')
main.add_module(roinput)

# Required setup modules
loadGearbox(main)
main.add_module("Geometry")

from rawdata import add_unpackers
add_unpackers(main)

printDataStore(path=main)

# Output events
output = register_module('RootOutput')
output.param('outputFileName', "unpacked.root")
main.add_module(output)

# Process events
process(main)
print(statistics)
