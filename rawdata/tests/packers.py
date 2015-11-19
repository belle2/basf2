#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from rawdata import add_packers, add_raw_output

main = create_path()

# input
input = register_module('RootInput')
input.param('inputFileNames', Belle2.FileSystem.findFile('rawdata/tests/digits.root'))
main.add_module(input)

# gearbox
main.add_module('Gearbox')

# geometry, needed by arich
main.add_module('Geometry')

# conversion from digits to raw data
# add_packers(main)

# output
# add_raw_output(main)

# process events
process(main)

# Print call statistics
print(statistics)
