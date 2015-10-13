#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from reconstruction import add_reconstruction

set_random_seed(12345)

main = create_path()

# Read the HLTTags from the HLT root file
# The gen+sim information will come from the GenSim root file
# which is loaded too by setting the parent level to 1
input = register_module('RootInput')
input.param('inputFileName', 'ProdHlt.root')
input.param('parentLevel', 1)
main.add_module(input)

# geometry parameter database
main.add_module(register_module('Gearbox'))

# detector geometry
main.add_module(register_module('Geometry'))

add_reconstruction(main)

# output
# will write all information (gen+sim+hlt+reco)
# to the final output file
output = register_module('RootOutput')
output.param('outputFileName', 'ProdGenSimReco.root')
main.add_module(output)

process(main)
