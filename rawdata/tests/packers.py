#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from rawdata import add_packers, add_raw_output

main = create_path()

# input
# main.add_module('RootInput', inputFileName=Belle2.FileSystem.findFile('rawdata/tests/digits.root'))
# main.add_module('RootInput', inputFileName='digits.root')
input = register_module('RootInput')
input.param('inputFileNames', 'digits.root')
main.add_module(input)

# gearbox
# main.add_module('Gearbox')
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# conversion from digits to raw data
add_packers(main)

# output
add_raw_output(main)

# process events
process(main)

# Print call statistics
print statistics
