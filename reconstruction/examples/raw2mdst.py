#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from rawdata import add_unpackers
from reconstruction import add_reconstruction, add_mdst_output
from HLTTrigger import add_HLT_Y4S

# create path
main = create_path()

# input
main.add_module('RootInput')

# gearbox and geometry
main.add_module('Gearbox')
main.add_module('Geometry')

# unpacking
add_unpackers(main)

# reconstruction
add_reconstruction(main)

# HLT physics trigger
add_HLT_Y4S(main)

# mdst output
add_mdst_output(main)

# process events and print call statistics
process(main)
print(statistics)
