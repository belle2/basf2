#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from reconstruction import add_reconstruction

from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main
from HLTTrigger import add_HLT_Y4S

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
input = register_module('RootInput')
input.param('inputFileName', 'ProdGenSim.root')
main.add_module(input)

# geometry parameter database
main.add_module(register_module('Gearbox'))

# detector geometry
main.add_module(register_module('Geometry'))

add_reconstruction(main)


# create charged tracks list
fillParticleList('pi+:HLT', '', path=main)

# create gamma list
fillParticleList('gamma:HLT', '', path=main)

add_HLT_Y4S(main)
# physicstrigger = register_module('PhysicsTrigger')
# main.add_module(physicstrigger)

# output
# store only the HLTTags, all other information will be discarded
output = register_module('RootOutput')
output.param('outputFileName', 'ProdHlt.root')
output.param('branchNames', 'HLTTags')
main.add_module(output)

process(main)
