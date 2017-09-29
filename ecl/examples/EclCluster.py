#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use EclDataAnalysis
# module to dump ECL related quantities in an ntuple
# starting from dst root file.
#
# The user should provide input and output root filnames
# as first and second argument respectively.
#
# Example steering file - 2014 Belle II Collaboration
########################################################

import os
import glob
from basf2 import *

if len(sys.argv) != 3:
    sys.exit('Must provide two input parameters: name of input file and name of output file')

# Create paths
main = create_path()

# Input file
# inputs = 'mdst_000544_prod00000001_task00000544.root'
inputs = sys.argv[1]

simpleinput = register_module('RootInput')
simpleinput.param('inputFileNames', inputs)
main.add_module(simpleinput)

# eclDataAnalysis module
eclanalysis = register_module('ECLClusterAnalysis')
eclanalysis.param('rootFileName', sys.argv[2])
eclanalysis.param('doTracking', 1)
main.add_module(eclanalysis)

process(main)
print(statistics)
