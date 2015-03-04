#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from simulation import add_simulation

if len(sys.argv) != 2:
    sys.exit('Must provide one input parameter: [input_Belle_MDST_file].\nA small example Belle MDST file can be downloaded from http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

inputBelleMDSTFile = sys.argv[1]

main = create_path()

# Input Module
input = register_module('PantherInput')
input.param('inputFileName', inputBelleMDSTFile)
main.add_module(input)

# Print out the contents of the DataStore
dump = register_module('PrintCollections')
main.add_module(dump)

# progress
progress = register_module('Progress')
main.add_module(progress)

process(main)

# Print call statistics
print statistics
