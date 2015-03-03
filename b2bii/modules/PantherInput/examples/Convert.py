#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from simulation import add_simulation
# from reconstruction import add_reconstruction
# import glob

argvs = sys.argv
argc = len(argvs)

main = create_path()

# Input Module
input = register_module('PantherInput')
input.param('inputFileName', argvs[1])
# input.param('inputFileName', '/pcidisk1-1/belle-data/e000055/HadronBJ/0307/on_resonance/00/HadronBJ-e000055r000002-b20070307_1108.mdst')
main.add_module(input)

# Store recoreds in ROOT
output = register_module('RootOutput')
output.param('outputFileName', argvs[2])
# output.param('outputFileName', 'hadronBJ-e55-run2.root')
main.add_module(output)

# progress
progress = register_module('Progress')
main.add_module(progress)

process(main)

# Print call statistics
print statistics
