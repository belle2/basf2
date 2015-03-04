#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from simulation import add_simulation

if len(sys.argv) != 3:
    sys.exit('Must provide two input parameters: [input_Belle_MDST_file] [output_BelleII_ROOT_file].\nA small example Belle MDST file can be downloaded from http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

inputBelleMDSTFile = sys.argv[1]
outputBelle2ROOTFile = sys.argv[2]

main = create_path()

# Input Module
input = register_module('PantherInput')
input.param('inputFileName', inputBelleMDSTFile)
main.add_module(input)

# Store the converted Belle II dataobjects in ROOT
output = register_module('RootOutput')
output.param('outputFileName', outputBelle2ROOTFile)
main.add_module(output)

# progress
progress = register_module('Progress')
main.add_module(progress)

process(main)

# Print call statistics
print statistics
