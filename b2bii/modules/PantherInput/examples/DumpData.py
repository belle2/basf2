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

# Dump Record
dump = register_module('PrintCollections')
# main.add_module(dump)

# progress
progress = register_module('Progress')
main.add_module(progress)

process(main)

# Print call statistics
print statistics
