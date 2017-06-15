#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
import glob

main = create_path()

main.add_module('RootInput', inputFileName=sys.argv[1])
add_simulation(main)
main.add_module('RootOutput', outputFileName=sys.argv[2])
main.add_module('Progress')
process(main)

# Print call statistics
print(statistics)
