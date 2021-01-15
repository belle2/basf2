# Add the cut module to the pat#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import modularAnalysis as ma
from rawdata import add_unpackers

# Create path
main = b2.create_path()

# Root input
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# Required setup modules
ma.loadGearbox(main)
main.add_module("Geometry")

add_unpackers(main)

ma.printDataStore(path=main)

# Output events
output = b2.register_module('RootOutput')
output.param('outputFileName', "unpacked.root")
main.add_module(output)

# Process events
b2.process(main)
print(b2.statistics)
